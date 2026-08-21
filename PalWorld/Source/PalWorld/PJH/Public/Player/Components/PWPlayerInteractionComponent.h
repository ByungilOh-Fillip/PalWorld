// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWPlayerInteractionComponent.generated.h"

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerInteractionComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Player|Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintPure, Category = "Player|Interaction")
	AActor* GetCurrentInteractable() const { return CurrentInteractable.Get(); }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Interaction", meta = (ClampMin = "0.0"))
	float SearchRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Interaction", meta = (ClampMin = "0.01"))
	float SearchInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Interaction|Highlight")
	bool bUseCustomDepthHighlight = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Interaction|Highlight")
	int32 HighlightStencilValue = 252;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentInteractable;

	float TimeUntilNextSearch = 0.f;

	UFUNCTION(Server, Reliable)
	void ServerInteractWithActor(AActor* TargetActor);

	void UpdateCurrentInteractable();
	AActor* FindBestInteractable() const;
	bool IsLocallyControlledOwner() const;
	bool IsInteractableAllowed(AActor* TargetActor) const;
	float GetInteractableScore(AActor* TargetActor) const;
	void SetActorHighlighted(AActor* TargetActor, bool bHighlighted) const;
};
