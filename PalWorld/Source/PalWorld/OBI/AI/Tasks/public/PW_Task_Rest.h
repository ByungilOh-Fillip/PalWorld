#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "PW_Task_Rest.generated.h"

USTRUCT(BlueprintType)
struct FPW_Task_RestInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> OwnerActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    class UAnimMontage* RestMontage = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float MinRestTime = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float MaxRestTime = 5.0f;

    UPROPERTY(VisibleAnywhere, Category = "Internal")
    float RandomDuration = 0.0f;
    
    UPROPERTY(VisibleAnywhere, Category = "Internal")
    float TimeElapsed = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "Internal")
    bool bIsEnding = false;
};

USTRUCT(meta = (DisplayName = "Rest (휴식 대기)", Category = "Pal|AI"))
struct PALWORLD_API FPWStateTreeTask_Rest : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    typedef FPW_Task_RestInstanceData InstanceDataType;
    
    virtual const UStruct* GetInstanceDataType() const override;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
