#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "PWPalAIController.generated.h"

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UStateTreeComponent;

/**
 * @class APWPalAIController
 * @brief 팰의 두뇌 역할을 하며 시각/청각 인지(AIPerception) 및 StateTree를 구동하는 컨트롤러
 */

USTRUCT(BlueprintType)
struct FST_PerceptionPayload
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|StateTree")
    AActor* TargetActor = nullptr;

};

UCLASS()
class PALWORLD_API APWPalAIController : public AAIController
{
    GENERATED_BODY()

public:
    APWPalAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

public:

    // 현재 쫓고 있거나 도망가고 있는 대상 (피격 또는 인지로 획득)
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Pal|AI")
    TObjectPtr<AActor> CurrentTargetActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|AI")
    TObjectPtr<UStateTreeComponent> StateTreeComponent;

protected:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|AI")
    TObjectPtr<UAIPerceptionComponent> PalPerceptionComponent;


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|AI")
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|AI")
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

    // StateTree 구동용 컴포넌트
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus const Stimulus);

};
