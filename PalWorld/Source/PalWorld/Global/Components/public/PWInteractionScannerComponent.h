#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWInteractionScannerComponent.generated.h"

class UPWInteractableTargetComponent;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWInteractionScannerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWInteractionScannerComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	AActor* GetCurrentInteractableActor() const { return CurrentInteractableActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	FText GetCurrentPrompt() const;

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction")
	bool TryInteract();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction", meta = (ClampMin = "0.0"))
	float ScanRadius = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction", meta = (ClampMin = "0.01"))
	float ScanIntervalSeconds = 0.1f;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentInteractableActor;

	float TimeUntilNextScan = 0.0f;

	UFUNCTION(Server, Reliable)
	void ServerTryInteract();

	void ScanForInteractables();
	AActor* FindBestInteractable() const;
	bool IsInteractableInRange(AActor* CandidateActor, const UPWInteractableTargetComponent* TargetComponent) const;
	bool IsBetterInteractable(AActor* CandidateActor, const UPWInteractableTargetComponent* CandidateComponent, AActor* BestActor, const UPWInteractableTargetComponent* BestComponent) const;
	bool ExecuteInteraction(AActor* InteractableActor) const;
	FVector GetScanOrigin() const;
	FVector GetViewDirection() const;
};
