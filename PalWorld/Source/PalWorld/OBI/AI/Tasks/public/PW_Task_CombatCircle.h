#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "PW_Task_CombatCircle.generated.h"

USTRUCT(BlueprintType)
struct FPW_Task_CombatCircleInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> OwnerActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float CircleRadius = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float MinDuration = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float MaxDuration = 5.0f;

    UPROPERTY(VisibleAnywhere, Category = "Internal")
    float RandomDuration = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "Internal")
    float TimeElapsed = 0.0f;
};

USTRUCT(meta = (DisplayName = "Combat Circle (원형 경계 배회)", Category = "Pal|AI"))
struct PALWORLD_API FPWStateTreeTask_CombatCircle : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    typedef FPW_Task_CombatCircleInstanceData InstanceDataType;
    
    virtual const UStruct* GetInstanceDataType() const override;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
