#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "PW_Eval_WorkFinder.generated.h"

USTRUCT(BlueprintType)
struct FPW_Eval_WorkFinderInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> OwnerActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Input")
    bool bIsNight = false;

    UPROPERTY(EditAnywhere, Category = "Input")
    bool bIsHungry = false;

    UPROPERTY(EditAnywhere, Category = "Output")
    TObjectPtr<AActor> TargetWorkActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Output")
    class UAnimMontage* FoundWorkMontage = nullptr;

    UPROPERTY(EditAnywhere, Category = "Output")
    bool bHasValidWork = false;
};

USTRUCT(meta = (DisplayName = "Work Finder (적성 작업 탐색)", Category = "Pal|AI"))
struct PALWORLD_API FPWStateTreeEvaluator_WorkFinder : public FStateTreeEvaluatorCommonBase
{
    GENERATED_BODY()

    typedef FPW_Eval_WorkFinderInstanceData InstanceDataType;
    
    virtual const UStruct* GetInstanceDataType() const override;

    virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
