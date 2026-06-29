// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWPlayerStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPWStaminaChangedSignature, float, CurrentStamina, float, MaxStamina, float, StaminaRatio);

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerStatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category = "Player|Stats|Stamina")
	FPWStaminaChangedSignature OnStaminaChanged;

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Stamina")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Stamina")
	float GetStaminaRatio() const;

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Stamina")
	float GetRollStaminaCost() const { return RollStaminaCost; }

	bool HasEnoughStamina(float Cost) const;
	bool CanStartSprint() const;
	bool TryConsumeStamina(float Cost);
	void SetSprintDrainActive(bool bNewIsSprintDrainActive);

private:
	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Player|Stats|Stamina", meta = (ClampMin = "0.0"))
	float MaxStamina = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina, VisibleInstanceOnly, Category = "Player|Stats|Stamina")
	float CurrentStamina = 100.f;

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

	UFUNCTION()
	void OnRep_CurrentStamina();

	void SetCurrentStamina(float NewCurrentStamina);
	void BroadcastStaminaChanged();
	void BlockStaminaRegen();
	bool ShouldRegenerateStamina() const;
};
