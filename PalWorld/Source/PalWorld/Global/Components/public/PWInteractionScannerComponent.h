#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "PWInteractionGuideTypes.h"
#include "PWInteractionScannerComponent.generated.h"

class UPWInteractableTargetComponent;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWInteractionScannerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWInteractionScannerComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	AActor* GetCurrentInteractableActor() const { return CurrentInteractableActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	FText GetCurrentPrompt() const;

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	void GetCurrentInteractionGuideActions(TArray<FPWInteractionGuideAction>& OutActions) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction")
	bool TryInteractByKey(FKey Key);

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction")
	bool TryInteractByActionId(FName ActionId);

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction")
	bool TryBeginHoldInteraction();

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction")
	void EndHoldInteraction();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction", meta = (ClampMin = "0.0"))
	float ScanRadius = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction", meta = (ClampMin = "0.01"))
	float ScanIntervalSeconds = 0.1f;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentInteractableActor;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentHoldInteractableActor;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentHoldGuideActor;

	FName CurrentHoldGuideActionId = NAME_None;
	float CurrentHoldGuideElapsedSeconds = 0.0f;
	float CurrentHoldGuideDurationSeconds = 1.0f;
	bool bHoldGuideProgressActive = false;

	float TimeUntilNextScan = 0.0f;

	UFUNCTION(Server, Reliable)
	void ServerTryInteract();

	UFUNCTION(Server, Reliable)
	void ServerTryInteractByKey(FName KeyName);

	UFUNCTION(Server, Reliable)
	void ServerTryInteractByActionId(FName ActionId);

	UFUNCTION(Server, Reliable)
	void ServerTryBeginHoldInteraction();

	UFUNCTION(Server, Reliable)
	void ServerEndHoldInteraction();

	void ScanForInteractables();
	AActor* FindBestInteractable() const;
	bool IsInteractableInRange(AActor* CandidateActor, const UPWInteractableTargetComponent* TargetComponent) const;
	bool IsBetterInteractable(AActor* CandidateActor, const UPWInteractableTargetComponent* CandidateComponent, AActor* BestActor, const UPWInteractableTargetComponent* BestComponent) const;
	bool ShouldUpdateLocalInteractionGuide() const;
	void SetCurrentInteractableActor(AActor* NewInteractableActor);
	void RefreshCurrentInteractionGuide();
	void HideInteractionGuide(AActor* InteractableActor) const;
	void ShowInteractionGuide(AActor* InteractableActor) const;
	void StartHoldGuideProgress(AActor* InteractableActor);
	void UpdateHoldGuideProgress(float DeltaTime);
	void StopHoldGuideProgress();
	bool GetHoldGuideAction(AActor* InteractableActor, FPWInteractionGuideAction& OutAction) const;
	void GetInteractionGuideActions(AActor* InteractableActor, TArray<FPWInteractionGuideAction>& OutActions) const;
	bool FindGuideActionByKey(AActor* InteractableActor, FName KeyName, FPWInteractionGuideAction& OutAction) const;
	bool FindGuideActionByActionId(AActor* InteractableActor, FName ActionId, FPWInteractionGuideAction& OutAction) const;
	bool ExecuteInteraction(AActor* InteractableActor) const;
	bool ExecuteInteractionAction(AActor* InteractableActor, FName ActionId) const;
	bool ExecuteInteractionByKey(AActor* InteractableActor, FName KeyName) const;
	bool ExecuteLocalInteraction(AActor* InteractableActor) const;
	bool ExecuteBeginHoldInteraction(AActor* InteractableActor);
	void ExecuteEndHoldInteraction(AActor* InteractableActor);
	FVector GetScanOrigin() const;
	FVector GetViewDirection() const;
};
