#include "Spawn/PW_WildPalSpawner.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "Interfaces/PW_WorldStateProvider.h"
#include "PWPalBase.h"
#include "Spawn/PW_PalSpawnPoolSubsystem.h"
#include "Spawn/PW_WildPalSpawnSubsystem.h"
#include "World/PW_WorldGameState.h"

APW_WildPalSpawner::APW_WildPalSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

void APW_WildPalSpawner::BeginPlay()
{
	Super::BeginPlay();

	DrawDebugSpawnRadius();

	if (CanRunAuthoritySpawner())
	{
		RegisterWithSubsystem();
	}
}

void APW_WildPalSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CanRunAuthoritySpawner())
	{
		if (UPW_WildPalSpawnSubsystem* SpawnSubsystem = GetWorld()->GetSubsystem<UPW_WildPalSpawnSubsystem>())
		{
			ReturnAllSpawnedPals(*SpawnSubsystem);
		}
		UnregisterFromSubsystem();
	}

	Super::EndPlay(EndPlayReason);
}

void APW_WildPalSpawner::EvaluateSpawner(UPW_WildPalSpawnSubsystem& SpawnSubsystem)
{
	if (!CanRunAuthoritySpawner())
	{
		return;
	}

	CleanupInvalidActivePals(SpawnSubsystem);

	const FVector SpawnerLocation = GetActorLocation();
	if (SpawnSubsystem.AreAllPlayersOutside(SpawnerLocation, DespawnDistance))
	{
		ReturnAllSpawnedPals(SpawnSubsystem);
		return;
	}

	if (!SpawnSubsystem.IsAnyPlayerWithin(SpawnerLocation, ActivationDistance))
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (World == nullptr || World->GetTimeSeconds() - LastSpawnTimeSeconds < RespawnIntervalSeconds)
	{
		return;
	}

	if (TrySpawnOnePal(SpawnSubsystem))
	{
		LastSpawnTimeSeconds = World->GetTimeSeconds();
	}
}

bool APW_WildPalSpawner::CanRunAuthoritySpawner() const
{
	const UWorld* World = GetWorld();
	return World != nullptr && World->GetAuthGameMode() != nullptr;
}

void APW_WildPalSpawner::RegisterWithSubsystem()
{
	if (UWorld* World = GetWorld())
	{
		if (UPW_WildPalSpawnSubsystem* SpawnSubsystem = World->GetSubsystem<UPW_WildPalSpawnSubsystem>())
		{
			SpawnSubsystem->RegisterSpawner(this);
		}
	}
}

void APW_WildPalSpawner::UnregisterFromSubsystem()
{
	if (UWorld* World = GetWorld())
	{
		if (UPW_WildPalSpawnSubsystem* SpawnSubsystem = World->GetSubsystem<UPW_WildPalSpawnSubsystem>())
		{
			SpawnSubsystem->UnregisterSpawner(this);
		}
	}
}

void APW_WildPalSpawner::CleanupInvalidActivePals(UPW_WildPalSpawnSubsystem& SpawnSubsystem)
{
	for (int32 Index = ActiveSpawnedPals.Num() - 1; Index >= 0; --Index)
	{
		if (!IsValid(ActiveSpawnedPals[Index]))
		{
			ActiveSpawnedPals.RemoveAtSwap(Index);
			SpawnSubsystem.ReleaseGlobalPal();
		}
	}
}

void APW_WildPalSpawner::ReturnAllSpawnedPals(UPW_WildPalSpawnSubsystem& SpawnSubsystem)
{
	UPW_PalSpawnPoolSubsystem* PoolSubsystem = GetWorld() != nullptr ? GetWorld()->GetSubsystem<UPW_PalSpawnPoolSubsystem>() : nullptr;
	if (PoolSubsystem == nullptr)
	{
		return;
	}

	for (int32 Index = ActiveSpawnedPals.Num() - 1; Index >= 0; --Index)
	{
		APWPalBase* PalActor = ActiveSpawnedPals[Index];
		ActiveSpawnedPals.RemoveAtSwap(Index);
		if (IsValid(PalActor))
		{
			PalActor->OnDestroyed.RemoveDynamic(this, &APW_WildPalSpawner::HandleSpawnedPalDestroyed);
			OnWildPalDespawned.Broadcast(PalActor);
			PoolSubsystem->ReturnPal(PalActor, this);
			SpawnSubsystem.ReleaseGlobalPal();
		}
	}
}

bool APW_WildPalSpawner::TrySpawnOnePal(UPW_WildPalSpawnSubsystem& SpawnSubsystem)
{
	if (ActiveSpawnedPals.Num() >= MaxActivePals)
	{
		return false;
	}

	const FPW_WildPalSpawnEntry* SpawnEntry = ChooseSpawnEntry();
	if (SpawnEntry == nullptr || SpawnEntry->PalClass == nullptr)
	{
		return false;
	}

	FTransform SpawnTransform;
	if (!FindSpawnTransform(SpawnSubsystem, SpawnTransform))
	{
		return false;
	}

	if (!SpawnSubsystem.TryReserveGlobalPal())
	{
		return false;
	}

	UPW_PalSpawnPoolSubsystem* PoolSubsystem = GetWorld() != nullptr ? GetWorld()->GetSubsystem<UPW_PalSpawnPoolSubsystem>() : nullptr;
	if (PoolSubsystem == nullptr)
	{
		SpawnSubsystem.ReleaseGlobalPal();
		return false;
	}

	APWPalBase* SpawnedPal = PoolSubsystem->AcquirePal(SpawnEntry->PalClass, SpawnTransform, this);
	if (SpawnedPal == nullptr)
	{
		SpawnSubsystem.ReleaseGlobalPal();
		return false;
	}

	SpawnedPal->SetNetCullDistanceSquared(FMath::Square(SpawnNetCullDistance));
	SpawnedPal->OnDestroyed.AddUniqueDynamic(this, &APW_WildPalSpawner::HandleSpawnedPalDestroyed);
	ActiveSpawnedPals.Add(SpawnedPal);
	OnWildPalSpawned.Broadcast(SpawnedPal);

	// TODO: Revisit Replication Graph or custom relevancy if active wild Pal count grows beyond the v1 budget.
	return true;
}

const FPW_WildPalSpawnEntry* APW_WildPalSpawner::ChooseSpawnEntry() const
{
	float TotalWeight = 0.0f;
	for (const FPW_WildPalSpawnEntry& Entry : SpawnEntries)
	{
		if (Entry.PalClass != nullptr && Entry.Weight > 0.0f && DoesEntryMatchWorldTime(Entry))
		{
			TotalWeight += Entry.Weight;
		}
	}

	if (TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	float Pick = FMath::FRandRange(0.0f, TotalWeight);
	for (const FPW_WildPalSpawnEntry& Entry : SpawnEntries)
	{
		if (Entry.PalClass == nullptr || Entry.Weight <= 0.0f || !DoesEntryMatchWorldTime(Entry))
		{
			continue;
		}

		Pick -= Entry.Weight;
		if (Pick <= 0.0f)
		{
			return &Entry;
		}
	}

	return nullptr;
}

bool APW_WildPalSpawner::DoesEntryMatchWorldTime(const FPW_WildPalSpawnEntry& Entry) const
{
	if (Entry.TimeRule == EPW_WildPalSpawnTimeRule::Always)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	const APW_WorldGameState* WorldGameState = World != nullptr ? World->GetGameState<APW_WorldGameState>() : nullptr;
	if (WorldGameState == nullptr)
	{
		return true;
	}

	const bool bIsNight = IPW_WorldStateProvider::Execute_IsNight(WorldGameState);
	return Entry.TimeRule == EPW_WildPalSpawnTimeRule::NightOnly ? bIsNight : !bIsNight;
}

bool APW_WildPalSpawner::FindSpawnTransform(UPW_WildPalSpawnSubsystem& SpawnSubsystem, FTransform& OutSpawnTransform) const
{
	UWorld* World = GetWorld();
	UNavigationSystemV1* NavigationSystem = World != nullptr ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	if (NavigationSystem == nullptr)
	{
		return false;
	}

	for (int32 AttemptIndex = 0; AttemptIndex < MaxSpawnLocationAttempts; ++AttemptIndex)
	{
		FNavLocation NavLocation;
		if (!NavigationSystem->GetRandomReachablePointInRadius(GetActorLocation(), SpawnRadius, NavLocation))
		{
			continue;
		}

		if (SpawnSubsystem.IsAnyPlayerWithin(NavLocation.Location, MinSpawnDistanceFromPlayer))
		{
			continue;
		}

		FRotator SpawnRotation = GetActorRotation();
		SpawnRotation.Yaw = FMath::FRandRange(0.0f, 360.0f);
		OutSpawnTransform = FTransform(SpawnRotation, NavLocation.Location);
		return true;
	}

	return false;
}

void APW_WildPalSpawner::DrawDebugSpawnRadius() const
{
	if (!bDrawDebugSpawnRadius)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	DrawDebugSphere(
		World,
		GetActorLocation(),
		SpawnRadius,
		32,
		FColor::Green,
		true,
		-1.0f,
		0,
		3.0f);
}

void APW_WildPalSpawner::HandleSpawnedPalDestroyed(AActor* DestroyedActor)
{
	APWPalBase* DestroyedPal = Cast<APWPalBase>(DestroyedActor);
	if (DestroyedPal == nullptr)
	{
		return;
	}

	const int32 RemovedCount = ActiveSpawnedPals.Remove(DestroyedPal);
	if (RemovedCount > 0)
	{
		OnWildPalDespawned.Broadcast(DestroyedPal);
		if (UPW_WildPalSpawnSubsystem* SpawnSubsystem = GetWorld() != nullptr ? GetWorld()->GetSubsystem<UPW_WildPalSpawnSubsystem>() : nullptr)
		{
			SpawnSubsystem->ReleaseGlobalPal();
		}
	}
}
