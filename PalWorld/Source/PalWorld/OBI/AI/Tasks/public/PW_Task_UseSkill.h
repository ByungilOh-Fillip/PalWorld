#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "PW_Task_UseSkill.generated.h"

USTRUCT(BlueprintType)
struct FPW_Task_UseSkillInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> OwnerActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> TargetActor = nullptr;

    // 내부에서 스킬 몽타주가 끝날 때까지 기다리기 위한 변수
    UPROPERTY(VisibleAnywhere, Category = "Internal")
    float MontageDuration = 0.0f;
    
    UPROPERTY(VisibleAnywhere, Category = "Internal")
    float TimeElapsed = 0.0f;
};

USTRUCT(meta = (DisplayName = "Use Random Skill (스킬 랜덤 사용)", Category = "Pal|AI"))
struct PALWORLD_API FPWStateTreeTask_UseSkill : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    typedef FPW_Task_UseSkillInstanceData InstanceDataType;
    
    virtual const UStruct* GetInstanceDataType() const override;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
