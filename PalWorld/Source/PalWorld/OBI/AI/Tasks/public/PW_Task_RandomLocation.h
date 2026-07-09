#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "PW_Task_RandomLocation.generated.h"

USTRUCT(BlueprintType)
struct FPW_Task_RandomLocationInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> OwnerActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float SearchRadius = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Output")
    FVector TargetLocation = FVector::ZeroVector;
};

USTRUCT(meta = (DisplayName = "Random Location (랜덤 위치 탐색)", Category = "Pal|AI"))
struct PALWORLD_API FPWStateTreeTask_RandomLocation : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    typedef FPW_Task_RandomLocationInstanceData InstanceDataType;
    
    virtual const UStruct* GetInstanceDataType() const override;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
