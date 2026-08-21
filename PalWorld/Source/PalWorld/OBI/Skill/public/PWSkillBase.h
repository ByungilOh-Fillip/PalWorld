#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PWSkillBase.generated.h"

UCLASS(Abstract, Blueprintable)
class PALWORLD_API APWSkillBase : public AActor
{
    GENERATED_BODY()
    
public:    
    APWSkillBase();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    // 스킬을 시전한 주체(Caster)
    UPROPERTY(BlueprintReadOnly, Category = "Skill")
    AActor* CasterActor;

    // 타겟(목표 대상)
    UPROPERTY(BlueprintReadOnly, Category = "Skill")
    AActor* TargetActor;

    // 스킬 위력(Power) 등 정보
    UPROPERTY(BlueprintReadOnly, Category = "Skill")
    float SkillPower;

    /**
     * @brief 스킬 초기화 및 실행 명령 (StateTree 또는 AI Controller에서 호출)
     * @param InCaster 스킬 시전자
     * @param InTarget 스킬 목표 (없을 수도 있음)
     * @param InPower 스킬 위력
     */
    UFUNCTION(BlueprintCallable, Category = "Skill")
    virtual void ExecuteSkill(AActor* InCaster, AActor* InTarget, float InPower);

    /**
     * @brief 스킬 동작이 끝나고 액터를 파괴할 때 호출
     */
    UFUNCTION(BlueprintCallable, Category = "Skill")
    virtual void FinishSkill();

protected:
    /**
     * @brief 데미지를 입히는 헬퍼 함수 (블루프린트 오버라이드 또는 C++ 직접 호출 가능)
     * @param HitActor 맞은 대상
     */
    UFUNCTION(BlueprintCallable, Category = "Skill")
    virtual void ApplySkillDamage(AActor* HitActor);
};
