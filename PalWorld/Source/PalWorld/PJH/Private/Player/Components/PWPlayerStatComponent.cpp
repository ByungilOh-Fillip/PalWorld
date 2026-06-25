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
		SetCurrentStamina(MaxStamina);
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

	if (bIsSprintDrainActive && SprintStaminaDrainPerSecond > 0.f)
	{
		SetCurrentStamina(CurrentStamina - SprintStaminaDrainPerSecond * DeltaTime);
		BlockStaminaRegen();

		if (CurrentStamina <= 0.f)
		{
			if (APWPlayerCharacter* PlayerCharacter = Cast<APWPlayerCharacter>(Owner))
			{
				PlayerCharacter->StopSprint();
			}
		}

		return;
	}

	if (ShouldRegenerateStamina())
	{
		SetCurrentStamina(CurrentStamina + StaminaRegenPerSecond * DeltaTime);
	}
}

void UPWPlayerStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPWPlayerStatComponent, MaxStamina);
	DOREPLIFETIME(UPWPlayerStatComponent, CurrentStamina);
}

float UPWPlayerStatComponent::GetStaminaRatio() const
{
	return MaxStamina > 0.f ? CurrentStamina / MaxStamina : 0.f;
}

bool UPWPlayerStatComponent::HasEnoughStamina(float Cost) const
{
	return Cost <= 0.f || CurrentStamina >= Cost;
}

bool UPWPlayerStatComponent::CanStartSprint() const
{
	return CurrentStamina >= MinSprintStartStamina;
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

	SetCurrentStamina(CurrentStamina - Cost);
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

	if (bIsSprintDrainActive)
	{
		BlockStaminaRegen();
	}
}

void UPWPlayerStatComponent::OnRep_CurrentStamina()
{
	CurrentStamina = FMath::Clamp(CurrentStamina, 0.f, MaxStamina);
	BroadcastStaminaChanged();
}

void UPWPlayerStatComponent::SetCurrentStamina(float NewCurrentStamina)
{
	const float ClampedStamina = FMath::Clamp(NewCurrentStamina, 0.f, MaxStamina);
	if (FMath::IsNearlyEqual(CurrentStamina, ClampedStamina))
	{
		return;
	}

	CurrentStamina = ClampedStamina;
	BroadcastStaminaChanged();
}

void UPWPlayerStatComponent::BroadcastStaminaChanged()
{
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, GetStaminaRatio());
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
	if (CurrentStamina >= MaxStamina || StaminaRegenPerSecond <= 0.f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	return !World || World->GetTimeSeconds() >= RegenBlockedUntilTime;
}
