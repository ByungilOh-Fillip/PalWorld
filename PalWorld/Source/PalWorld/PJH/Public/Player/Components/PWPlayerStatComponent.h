// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StatusComponent.h"
#include "PWPlayerStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPWHealthChangedSignature, float, CurrentHealth, float, MaxHealth, float, HealthRatio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPWStaminaChangedSignature, float, CurrentStamina, float, MaxStamina, float, StaminaRatio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPWHungerChangedSignature, float, CurrentHunger, float, MaxHunger, float, HungerRatio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPWPlayerSurvivalStatsChangedSignature);

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerStatComponent : public UStatusComponent
{
	GENERATED_BODY()

public:
	UPWPlayerStatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Player|Stats|Health")
	FPWHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Player|Stats|Stamina")
	FPWStaminaChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Player|Stats|Hunger")
	FPWHungerChangedSignature OnHungerChanged;

	UPROPERTY(BlueprintAssignable, Category = "Player|Stats")
	FPWPlayerSurvivalStatsChangedSignature OnSurvivalStatsChanged;

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Health")
	float GetCurrentHealth() const { return CurrentHP; }

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Health")
	float GetMaxHealth() const { return MaxHP; }

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Health")
	float GetHealthRatio() const;

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Stamina")
	float GetCurrentStamina() const { return CurrentSP; }

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Stamina")
	float GetMaxStamina() const { return MaxSP; }

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Stamina")
	float GetStaminaRatio() const;

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Stamina")
	float GetRollStaminaCost() const { return RollStaminaCost; }

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Hunger")
	float GetCurrentHunger() const { return CurrentHunger; }

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Hunger")
	float GetMaxHunger() const { return MaxHunger; }

	UFUNCTION(BlueprintPure, Category = "Player|Stats|Hunger")
	float GetHungerRatio() const;

	UFUNCTION(BlueprintCallable, Category = "Player|Stats|Health")
	bool ApplyHealthDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Player|Stats|Health")
	bool RestoreHealth(float RestoreAmount);

	UFUNCTION(BlueprintCallable, Category = "Player|Stats|Hunger")
	bool ConsumeHunger(float HungerAmount);

	UFUNCTION(BlueprintCallable, Category = "Player|Stats|Hunger")
	bool RestoreHunger(float RestoreAmount);

	bool HasEnoughStamina(float Cost) const;
	bool CanStartSprint() const;
	bool TryConsumeStamina(float Cost);
	void SetSprintDrainActive(bool bNewIsSprintDrainActive);

protected:
	virtual void HandleCurrentHPChanged() override;
	virtual void HandleCurrentSPChanged() override;
	virtual void HandleCurrentHungerChanged() override;

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

	UPROPERTY(EditDefaultsOnly, Category = "Player|Stats|Hunger", meta = (ClampMin = "0.0"))
	float HungerDrainPerSecond = 0.03f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Stats|Hunger")
	bool bEnableHungerDrain = true;

	bool bIsSprintDrainActive = false;
	float RegenBlockedUntilTime = 0.f;

	void SetCurrentHealth(float NewCurrentHealth);
	void SetCurrentStamina(float NewCurrentStamina);
	void SetCurrentHunger(float NewCurrentHunger);
	void BroadcastHealthChanged();
	void BroadcastStaminaChanged();
	void BroadcastHungerChanged();
	void BroadcastSurvivalStatsChanged();
	void BlockStaminaRegen();
	bool ShouldRegenerateStamina() const;
};
