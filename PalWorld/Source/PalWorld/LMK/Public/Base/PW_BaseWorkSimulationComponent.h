#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BaseTypes.h"
#include "Components/ActorComponent.h"
#include "PW_BaseWorkSimulationComponent.generated.h"

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_BaseWorkSimulationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_BaseWorkSimulationComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Work")
	bool AddWorkState(const FPW_BaseWorkState& WorkState);

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Work")
	void SimulateUntilNow();

	UFUNCTION(BlueprintPure, Category = "PW|Base|Work")
	const TArray<FPW_BaseWorkState>& GetWorkStates() const { return WorkStates; }

protected:
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Base|Work")
	TArray<FPW_BaseWorkState> WorkStates;
};
