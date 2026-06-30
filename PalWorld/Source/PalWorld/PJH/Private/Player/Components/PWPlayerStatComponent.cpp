// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerStatComponent.h"

#include "Engine/World.h"
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
		SetCurrentStamina(MaxSP);
	}

	BroadcastStaminaChanged();
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

		return;
	}

	if (ShouldRegenerateStamina())
	{
		SetCurrentStamina(CurrentSP + StaminaRegenPerSecond * DeltaTime);
	}
}

float UPWPlayerStatComponent::GetStaminaRatio() const
{
	return MaxSP > 0.f ? CurrentSP / MaxSP : 0.f;
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

void UPWPlayerStatComponent::HandleCurrentSPChanged()
{
	Super::HandleCurrentSPChanged();

	CurrentSP = FMath::Clamp(CurrentSP, 0.f, MaxSP);
	BroadcastStaminaChanged();
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

void UPWPlayerStatComponent::BroadcastStaminaChanged()
{
	OnStaminaChanged.Broadcast(CurrentSP, MaxSP, GetStaminaRatio());
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

bool UPWPlayerStatComponent::ShouldRegenerateStamina() const
{
	if (CurrentSP >= MaxSP || StaminaRegenPerSecond <= 0.f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	return !World || World->GetTimeSeconds() >= RegenBlockedUntilTime;
}
