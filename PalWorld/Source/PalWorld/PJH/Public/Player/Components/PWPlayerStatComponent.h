// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StatusComponent.h"
#include "PWPlayerStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPWStaminaChangedSignature, float, CurrentStamina, float, MaxStamina, float, StaminaRatio);

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerStatComponent : public UStatusComponent
{
	GENERATED_BODY()

public:
	UPWPlayerStatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Player|Stats|Stamina")
	FPWStaminaChangedSignature OnStaminaChanged;

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Stamina")
	float GetCurrentStamina() const { return CurrentSP; }

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Stamina")
	float GetMaxStamina() const { return MaxSP; }

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Stamina")
	float GetStaminaRatio() const;

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Stamina")
	float GetRollStaminaCost() const { return RollStaminaCost; }

	bool HasEnoughStamina(float Cost) const;
	bool CanStartSprint() const;
	bool TryConsumeStamina(float Cost);
	void SetSprintDrainActive(bool bNewIsSprintDrainActive);

protected:
	virtual void HandleCurrentSPChanged() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Player|Stats|Stamina", meta = (ClampMin = "0.0"))
	float StaminaRegenPerSecond = 18.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Stats|Stamina", meta = (ClampMin = "0.0"))
	float StaminaRegenDelay = 0.75f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Stats|Stamina", meta = (ClampMin = "0.0"))
	float RollStaminaCost = 22.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Stats|Stamina", meta = (ClampMin = "0.0"))
	float SprintStaminaDrainPerSecond = 14.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Stats|Stamina", meta = (ClampMin = "0.0"))
	float MinSprintStartStamina = 5.f;

	bool bIsSprintDrainActive = false;
	float RegenBlockedUntilTime = 0.f;

	void SetCurrentStamina(float NewCurrentStamina);
	void BroadcastStaminaChanged();
	void BlockStaminaRegen();
	bool ShouldRegenerateStamina() const;
};
