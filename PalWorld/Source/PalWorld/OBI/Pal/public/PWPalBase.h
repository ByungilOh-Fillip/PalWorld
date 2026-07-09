#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Character.h"
#include "PWPalBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateTagChanged, FGameplayTag, Tag, bool, bAdded);

class UStatusComponent;
class UPWSkillComponent;

UCLASS()
class PALWORLD_API APWPalBase : public ACharacter, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    APWPalBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintPure, Category = "Pal|Components")
    UPWSkillComponent* GetSkillComponent() const { return SkillComponent; }

    // 1. 엔진이 이 캐릭터의 태그를 물어볼 때 대답해주는 필수 인터페이스 함수
    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

    // 네트워크 리플리케이션 등록
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 2. 실제로 태그가 담길 주머니 (블루프린트에서도 볼 수 있게 노출, 멀티플레이 동기화)
    UPROPERTY(ReplicatedUsing = OnRep_ActiveTags, EditAnywhere, BlueprintReadWrite, Category = "StateTags")
    FGameplayTagContainer ActiveTags;

    // 태그 동기화 시 클라이언트에서 호출될 OnRep 함수 (이전 상태를 매개변수로 받아 Diff 계산)
    UFUNCTION()
    void OnRep_ActiveTags(const FGameplayTagContainer& PreviousTags);

    // 태그가 추가/삭제될 때 호출될 다이내믹 멀티캐스트 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "StateTags")
    FOnStateTagChanged OnStateTagChangedDelegate;

    // 3. StateTree에서 상태가 변할 때 태그를 넣고 뺄 수 있도록 도와주는 함수
    UFUNCTION(BlueprintCallable, Category = "StateTags")
    void AddStateTag(FGameplayTag TagToAdd);

    UFUNCTION(BlueprintCallable, Category = "StateTags")
    void RemoveStateTag(FGameplayTag TagToRemove);

    // 팰 고유 데이터 에셋 (피격 몽타주 등 자동화 관리용)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Data")
    class UPWPalDataAsset* PalDataAsset;

    // 거점 (BaseCamp) 정보 (거점에 소속된 팰일 경우 이동 반경 제한용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|BaseCamp")
    TObjectPtr<AActor> BaseCampActor;

    // 거점 활동 반경
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|BaseCamp")
    float BaseCampRadius = 3000.0f;
    
    // 현재 이 팰이 거점에 소속되어 있는지 여부
    UFUNCTION(BlueprintCallable, Category = "Pal|BaseCamp")
    bool IsAssignedToBaseCamp() const { return BaseCampActor != nullptr; }

protected:
    // 피격 애니메이션 재생 함수 (Perception 감지나 데미지 처리 시 호출)
    UFUNCTION(BlueprintCallable, Category = "Pal|Animation")
    void PlayHitReaction();

protected:
    // 태그 변화에 따라 이동 속도(MaxWalkSpeed)를 변경하는 내부 이벤트 함수
    UFUNCTION()
    void HandleStateTagChanged(FGameplayTag Tag, bool bAdded);

protected:
    // 팰 상태 관리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|Components")
    TObjectPtr<UStatusComponent> StatusComponent;

    // 팰 스킬 및 적성 관리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|Components")
    TObjectPtr<UPWSkillComponent> SkillComponent;
};
