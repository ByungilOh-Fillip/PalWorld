// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/Resources/PWGatherableResourceActor.h"

#include "Net/UnrealNetwork.h"

APWGatherableResourceActor::APWGatherableResourceActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
}

void APWGatherableResourceActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentHealth = MaxHealth;
	}
}

void APWGatherableResourceActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APWGatherableResourceActor, CurrentHealth);
}

bool APWGatherableResourceActor::ApplyGatherDamage(AActor* GatherInstigator, float DamageAmount, EPWToolType ToolType)
{
	if (!HasAuthority() || IsDepleted() || DamageAmount <= 0.f)
	{
		return false;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);
	const float AppliedDamage = PreviousHealth - CurrentHealth;
	if (AppliedDamage <= 0.f)
	{
		return false;
	}

	DamageSinceLastDrop += AppliedDamage;

	int32 DropIntervalCount = 0;
	while (DamageSinceLastDrop >= DropDamageInterval)
	{
		DamageSinceLastDrop -= DropDamageInterval;
		++DropIntervalCount;
	}

	BP_OnGatherDamaged(GatherInstigator, AppliedDamage, ToolType, CurrentHealth);

	if (DropIntervalCount > 0)
	{
		GenerateDrops(GatherInstigator, DropIntervalCount);
	}

	if (IsDepleted())
	{
		DepleteResource(GatherInstigator);
	}

	ForceNetUpdate();
	return true;
}

void APWGatherableResourceActor::OnRep_CurrentHealth()
{
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);
}

void APWGatherableResourceActor::GenerateDrops(AActor* GatherInstigator, int32 IntervalCount)
{
	const int32 DropCount = FMath::Max(IntervalCount, 0) * FMath::Max(DropCountPerInterval, 1);
	if (DropCount <= 0)
	{
		return;
	}

	BP_OnGatherDropGenerated(GatherInstigator, DropType, DropCount);

	UE_LOG(LogTemp, Log, TEXT("[PWGather] Drop generated. Resource=%s Drop=%s Count=%d"),
		*GetName(),
		*UEnum::GetValueAsString(DropType),
		DropCount);
}

void APWGatherableResourceActor::DepleteResource(AActor* GatherInstigator)
{
	BP_OnResourceDepleted(GatherInstigator);

	UE_LOG(LogTemp, Log, TEXT("[PWGather] Resource depleted. Resource=%s"), *GetName());

	if (bDestroyWhenDepleted)
	{
		Destroy();
	}
}
