#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "PW_Task_CalcFleeLocation.generated.h"

USTRUCT(BlueprintType)
struct FPW_Task_CalcFleeLocationInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> OwnerActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> AttackerActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float FleeDistance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Output")
    FVector TargetLocation = FVector::ZeroVector;
};

USTRUCT(meta = (DisplayName = "Calc Flee Location (도주 위치 계산)", Category = "Pal|AI"))
struct PALWORLD_API FPWStateTreeTask_CalcFleeLocation : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    typedef FPW_Task_CalcFleeLocationInstanceData InstanceDataType;
    
    virtual const UStruct* GetInstanceDataType() const override;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
