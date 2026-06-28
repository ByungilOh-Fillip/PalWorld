#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PWStatusEffectManager.generated.h"

class UStatusComponent;

/**
 * @class UPWStatusEffectManager
 * @brief 캐릭터(Pal, Player 등)에 부착되어 상태이상(GameplayTag) 부여, 해제, 틱(도트 데미지 등)을 전담 관리하는 컴포넌트.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PALWORLD_API UPWStatusEffectManager : public UActorComponent
{
    GENERATED_BODY()

public:    
    UPWStatusEffectManager();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** 상태 이상 태그를 부여합니다. 대상 검증 로직이 포함됩니다. */
    UFUNCTION(BlueprintCallable, Category = "Pal|StatusEffect")
    void ApplyStatusEffect(FGameplayTag EffectTag);

    /** 상태 이상 태그를 제거합니다. */
    UFUNCTION(BlueprintCallable, Category = "Pal|StatusEffect")
    void RemoveStatusEffect(FGameplayTag EffectTag);

    /** 특정 상태 이상 태그를 가지고 있는지 확인합니다. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pal|StatusEffect")
    bool HasStatusEffect(FGameplayTag EffectTag) const;

protected:
    // 부착된 대상의 StatusComponent 캐싱
    UPROPERTY(Transient)
    UStatusComponent* OwnerStatusComponent;
};
