#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "PW_Task_Kite.generated.h"

USTRUCT(BlueprintType)
struct FPW_Task_KiteInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> OwnerActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    class UAnimMontage* KiteMontage = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter|Physics")
    float JumpBackwardForce = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Parameter|Physics")
    float JumpUpwardForce = 400.0f;

    UPROPERTY(EditAnywhere, Category = "Parameter|Physics")
    int32 NumberOfJumps = 2; // 몇 번 연속으로 뛸 것인지

    UPROPERTY(VisibleAnywhere, Category = "Internal")
    float MontageDuration = 0.0f;
    
    UPROPERTY(VisibleAnywhere, Category = "Internal")
    float TimeSinceLastJump = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "Internal")
    int32 JumpsPerformed = 0;
};

USTRUCT(meta = (DisplayName = "Kite (뒤로 회피)", Category = "Pal|AI"))
struct PALWORLD_API FPWStateTreeTask_Kite : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    typedef FPW_Task_KiteInstanceData InstanceDataType;
    
    virtual const UStruct* GetInstanceDataType() const override;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
