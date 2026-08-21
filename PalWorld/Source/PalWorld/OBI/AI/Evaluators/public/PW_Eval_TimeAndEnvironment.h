#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "PW_Eval_TimeAndEnvironment.generated.h"

USTRUCT(BlueprintType)
struct FPW_Eval_TimeAndEnvironmentInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Output")
    bool bIsNight = false;
};

USTRUCT(meta = (DisplayName = "Time And Environment (낮/밤 검사)", Category = "Pal|AI"))
struct PALWORLD_API FPWStateTreeEvaluator_TimeAndEnv : public FStateTreeEvaluatorCommonBase
{
    GENERATED_BODY()

    typedef FPW_Eval_TimeAndEnvironmentInstanceData InstanceDataType;
    
    virtual const UStruct* GetInstanceDataType() const override;
    void TreeStart(FStateTreeExecutionContext& Context) const;

    virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
