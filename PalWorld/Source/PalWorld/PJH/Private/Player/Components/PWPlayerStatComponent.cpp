// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerStatComponent.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Player/Core/PWPlayerCharacter.h"

UPWPlayerStatComponent::UPWPlayerStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UPWPlayerStatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		SetCurrentHealth(MaxHP);
		SetCurrentShield(MaxShield);
		SetCurrentStamina(MaxSP);
		SetCurrentHunger(MaxHunger);
	}

	BroadcastHealthChanged();
	BroadcastShieldChanged();
	BroadcastStaminaChanged();
	BroadcastHungerChanged();
	BroadcastSurvivalStatsChanged();
}

void UPWPlayerStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPWPlayerStatComponent, CurrentShield);
	DOREPLIFETIME(UPWPlayerStatComponent, MaxShield);
}

void UPWPlayerStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	APWPlayerCharacter* PlayerCharacter = Cast<APWPlayerCharacter>(Owner);
	const bool bShouldDrainSprintStamina = bIsSprintDrainActive
		&& SprintStaminaDrainPerSecond > 0.f
		&& PlayerCharacter
		&& PlayerCharacter->ShouldDrainSprintStamina();

	if (bShouldDrainSprintStamina)
	{
		SetCurrentStamina(CurrentSP - SprintStaminaDrainPerSecond * DeltaTime);
		BlockStaminaRegen();

		if (CurrentSP <= 0.f)
		{
			PlayerCharacter->StopSprint();
		}
	}
	else if (ShouldRegenerateStamina())
	{
		SetCurrentStamina(CurrentSP + StaminaRegenPerSecond * DeltaTime);
	}

	if (bEnableHungerDrain && HungerDrainPerSecond > 0.f)
	{
		SetCurrentHunger(CurrentHunger - HungerDrainPerSecond * DeltaTime);
	}

	ApplyHealthRegen(DeltaTime);
	ApplyShieldRegen(DeltaTime);
	ApplyStarvationDamage(DeltaTime);
}

float UPWPlayerStatComponent::GetHealthRatio() const
{
	return MaxHP > 0.f ? CurrentHP / MaxHP : 0.f;
}

float UPWPlayerStatComponent::GetShieldRatio() const
{
	return MaxShield > 0.f ? CurrentShield / MaxShield : 0.f;
}

float UPWPlayerStatComponent::GetStaminaRatio() const
{
	return MaxSP > 0.f ? CurrentSP / MaxSP : 0.f;
}

float UPWPlayerStatComponent::GetHungerRatio() const
{
	return MaxHunger > 0.f ? CurrentHunger / MaxHunger : 0.f;
}

bool UPWPlayerStatComponent::ApplyHealthDamage(float DamageAmount)
{
	if (DamageAmount <= 0.f)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	const float ShieldDamage = FMath::Min(CurrentShield, DamageAmount);
	if (ShieldDamage > 0.f)
	{
		SetCurrentShield(CurrentShield - ShieldDamage);
	}

	const float RemainingDamage = DamageAmount - ShieldDamage;
	if (RemainingDamage > 0.f)
	{
		SetCurrentHealth(CurrentHP - RemainingDamage);
	}

	BlockShieldRegen();
	return true;
}

bool UPWPlayerStatComponent::ApplyDirectHealthDamage(float DamageAmount)
{
	if (DamageAmount <= 0.f)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	SetCurrentHealth(CurrentHP - DamageAmount);
	BlockShieldRegen();
	return true;
}

bool UPWPlayerStatComponent::RestoreHealth(float RestoreAmount)
{
	if (RestoreAmount <= 0.f)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	SetCurrentHealth(CurrentHP + RestoreAmount);
	return true;
}

bool UPWPlayerStatComponent::RestoreShield(float RestoreAmount)
{
	if (RestoreAmount <= 0.f)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	SetCurrentShield(CurrentShield + RestoreAmount);
	return true;
}

bool UPWPlayerStatComponent::ConsumeShield(float ShieldAmount)
{
	if (ShieldAmount <= 0.f)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || CurrentShield < ShieldAmount)
	{
		return false;
	}

	SetCurrentShield(CurrentShield - ShieldAmount);
	BlockShieldRegen();
	return true;
}

void UPWPlayerStatComponent::SetShieldCapacity(float NewMaxShield, bool bFillShield)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	const float ClampedMaxShield = FMath::Max(0.f, NewMaxShield);
	const bool bMaxShieldChanged = !FMath::IsNearlyEqual(MaxShield, ClampedMaxShield);
	MaxShield = ClampedMaxShield;

	if (bFillShield)
	{
		SetCurrentShield(MaxShield);
	}
	else
	{
		SetCurrentShield(FMath::Min(CurrentShield, MaxShield));
	}

	if (bMaxShieldChanged)
	{
		BroadcastShieldChanged();
	}
}

bool UPWPlayerStatComponent::ConsumeHunger(float HungerAmount)
{
	if (HungerAmount <= 0.f)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	SetCurrentHunger(CurrentHunger - HungerAmount);
	return true;
}

bool UPWPlayerStatComponent::RestoreHunger(float RestoreAmount)
{
	if (RestoreAmount <= 0.f)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	SetCurrentHunger(CurrentHunger + RestoreAmount);
	return true;
}

bool UPWPlayerStatComponent::HasEnoughStamina(float Cost) const
{
	return Cost <= 0.f || CurrentSP >= Cost;
}

bool UPWPlayerStatComponent::CanStartSprint() const
{
	return CurrentSP >= MinSprintStartStamina;
}

bool UPWPlayerStatComponent::TryConsumeStamina(float Cost)
{
	if (Cost <= 0.f)
	{
		return true;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || !HasEnoughStamina(Cost))
	{
		return false;
	}

	SetCurrentStamina(CurrentSP - Cost);
	BlockStaminaRegen();
	return true;
}

void UPWPlayerStatComponent::SetSprintDrainActive(bool bNewIsSprintDrainActive)
{
	if (bIsSprintDrainActive == bNewIsSprintDrainActive)
	{
		return;
	}

	bIsSprintDrainActive = bNewIsSprintDrainActive;
}

void UPWPlayerStatComponent::OnRep_CurrentShield()
{
	CurrentShield = FMath::Clamp(CurrentShield, 0.f, MaxShield);
	BroadcastShieldChanged();
}

void UPWPlayerStatComponent::OnRep_MaxShield()
{
	CurrentShield = FMath::Clamp(CurrentShield, 0.f, MaxShield);
	BroadcastShieldChanged();
}

void UPWPlayerStatComponent::HandleCurrentHPChanged()
{
	Super::HandleCurrentHPChanged();

	CurrentHP = FMath::Clamp(CurrentHP, 0.f, MaxHP);
	BroadcastHealthChanged();
}

void UPWPlayerStatComponent::HandleCurrentSPChanged()
{
	Super::HandleCurrentSPChanged();

	CurrentSP = FMath::Clamp(CurrentSP, 0.f, MaxSP);
	BroadcastStaminaChanged();
}

void UPWPlayerStatComponent::HandleCurrentHungerChanged()
{
	Super::HandleCurrentHungerChanged();

	CurrentHunger = FMath::Clamp(CurrentHunger, 0.f, MaxHunger);
	BroadcastHungerChanged();
}

void UPWPlayerStatComponent::SetCurrentHealth(float NewCurrentHealth)
{
	const float ClampedHealth = FMath::Clamp(NewCurrentHealth, 0.f, MaxHP);
	if (FMath::IsNearlyEqual(CurrentHP, ClampedHealth))
	{
		return;
	}

	CurrentHP = ClampedHealth;
	BroadcastHealthChanged();
}

void UPWPlayerStatComponent::SetCurrentShield(float NewCurrentShield)
{
	const float ClampedShield = FMath::Clamp(NewCurrentShield, 0.f, MaxShield);
	if (FMath::IsNearlyEqual(CurrentShield, ClampedShield))
	{
		return;
	}

	CurrentShield = ClampedShield;
	BroadcastShieldChanged();
}

void UPWPlayerStatComponent::SetCurrentStamina(float NewCurrentStamina)
{
	const float ClampedStamina = FMath::Clamp(NewCurrentStamina, 0.f, MaxSP);
	if (FMath::IsNearlyEqual(CurrentSP, ClampedStamina))
	{
		return;
	}

	CurrentSP = ClampedStamina;
	BroadcastStaminaChanged();
}

void UPWPlayerStatComponent::SetCurrentHunger(float NewCurrentHunger)
{
	const float ClampedHunger = FMath::Clamp(NewCurrentHunger, 0.f, MaxHunger);
	if (FMath::IsNearlyEqual(CurrentHunger, ClampedHunger))
	{
		return;
	}

	CurrentHunger = ClampedHunger;
	BroadcastHungerChanged();
}

void UPWPlayerStatComponent::ApplyShieldRegen(float DeltaTime)
{
	if (!ShouldRegenerateShield())
	{
		return;
	}

	SetCurrentShield(CurrentShield + ShieldRegenPerSecond * DeltaTime);
}

void UPWPlayerStatComponent::ApplyHealthRegen(float DeltaTime)
{
	if (!ShouldRegenerateHealth())
	{
		return;
	}

	const float MissingHealth = MaxHP - CurrentHP;
	const float HealthToRestore = FMath::Min(MissingHealth, HealthRegenPerSecond * DeltaTime);
	if (HealthToRestore <= 0.f)
	{
		return;
	}

	SetCurrentHealth(CurrentHP + HealthToRestore);

	const float ExtraHungerCost = HealthToRestore * HealthRegenHungerCostPerHealth;
	if (ExtraHungerCost > 0.f)
	{
		SetCurrentHunger(CurrentHunger - ExtraHungerCost);
	}
}

void UPWPlayerStatComponent::ApplyStarvationDamage(float DeltaTime)
{
	if (CurrentHunger > 0.f || CurrentHP <= 0.f || StarvationHealthDamagePerSecond <= 0.f)
	{
		return;
	}

	ApplyDirectHealthDamage(StarvationHealthDamagePerSecond * DeltaTime);
}

void UPWPlayerStatComponent::BroadcastHealthChanged()
{
	OnHealthChanged.Broadcast(CurrentHP, MaxHP, GetHealthRatio());
	BroadcastSurvivalStatsChanged();
}

void UPWPlayerStatComponent::BroadcastShieldChanged()
{
	OnShieldChanged.Broadcast(CurrentShield, MaxShield, GetShieldRatio());
	BroadcastSurvivalStatsChanged();
}

void UPWPlayerStatComponent::BroadcastStaminaChanged()
{
	OnStaminaChanged.Broadcast(CurrentSP, MaxSP, GetStaminaRatio());
	BroadcastSurvivalStatsChanged();
}

void UPWPlayerStatComponent::BroadcastHungerChanged()
{
	OnHungerChanged.Broadcast(CurrentHunger, MaxHunger, GetHungerRatio());
	BroadcastSurvivalStatsChanged();
}

void UPWPlayerStatComponent::BroadcastSurvivalStatsChanged()
{
	OnSurvivalStatsChanged.Broadcast();
}

void UPWPlayerStatComponent::BlockStaminaRegen()
{
	const UWorld* World = GetWorld();
	if (!World || StaminaRegenDelay <= 0.f)
	{
		return;
	}

	RegenBlockedUntilTime = World->GetTimeSeconds() + StaminaRegenDelay;
}

void UPWPlayerStatComponent::BlockShieldRegen()
{
	const UWorld* World = GetWorld();
	if (!World || ShieldRegenDelay <= 0.f)
	{
		ShieldRegenBlockedUntilTime = 0.f;
		return;
	}

	ShieldRegenBlockedUntilTime = World->GetTimeSeconds() + ShieldRegenDelay;
}

bool UPWPlayerStatComponent::ShouldRegenerateStamina() const
{
	if (CurrentSP >= MaxSP || StaminaRegenPerSecond <= 0.f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	return !World || World->GetTimeSeconds() >= RegenBlockedUntilTime;
}

bool UPWPlayerStatComponent::ShouldRegenerateShield() const
{
	if (MaxShield <= 0.f || CurrentShield >= MaxShield || ShieldRegenPerSecond <= 0.f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	return !World || World->GetTimeSeconds() >= ShieldRegenBlockedUntilTime;
}

bool UPWPlayerStatComponent::ShouldRegenerateHealth() const
{
	if (CurrentHP >= MaxHP || CurrentHP <= 0.f || HealthRegenPerSecond <= 0.f)
	{
		return false;
	}

	if (MaxHunger <= 0.f || CurrentHunger <= 0.f)
	{
		return false;
	}

	return GetHungerRatio() >= HealthRegenMinHungerRatio;
}
