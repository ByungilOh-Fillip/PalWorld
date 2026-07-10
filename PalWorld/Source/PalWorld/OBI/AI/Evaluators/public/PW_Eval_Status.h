#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "PW_Eval_Status.generated.h"

USTRUCT(BlueprintType)
struct FPW_Eval_StatusInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> OwnerActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Output")
    bool bIsHungry = false;
};

USTRUCT(meta = (DisplayName = "Status Check (상태 검사)", Category = "Pal|AI"))
struct PALWORLD_API FPWStateTreeEvaluator_Status : public FStateTreeEvaluatorCommonBase
{
    GENERATED_BODY()

    typedef FPW_Eval_StatusInstanceData InstanceDataType;
    
    virtual const UStruct* GetInstanceDataType() const override;
    void TreeStart(FStateTreeExecutionContext& Context) const;

    virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
