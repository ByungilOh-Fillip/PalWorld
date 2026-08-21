#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "PW_Task_Interact.generated.h"

USTRUCT(BlueprintType)
struct FPW_Task_InteractInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> OwnerActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> TargetInteractActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    class UAnimMontage* InteractMontage = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Internal")
    float MontageDuration = 0.0f;
    
    UPROPERTY(VisibleAnywhere, Category = "Internal")
    float TimeElapsed = 0.0f;
};

USTRUCT(meta = (DisplayName = "Interact (상호작용 몽타주)", Category = "Pal|AI"))
struct PALWORLD_API FPWStateTreeTask_Interact : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    typedef FPW_Task_InteractInstanceData InstanceDataType;
    
    virtual const UStruct* GetInstanceDataType() const override;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
