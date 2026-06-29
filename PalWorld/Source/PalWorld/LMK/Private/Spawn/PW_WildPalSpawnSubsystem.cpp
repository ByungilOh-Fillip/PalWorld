#include "Spawn/PW_WildPalSpawnSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Spawn/PW_WildPalSpawner.h"

void UPW_WildPalSpawnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!CanRunAuthoritySpawn())
	{
		return;
	}

	InWorld.GetTimerManager().SetTimer(
		EvaluationTimerHandle,
		this,
		&UPW_WildPalSpawnSubsystem::EvaluateSpawners,
		EvaluationIntervalSeconds,
		true);
}

void UPW_WildPalSpawnSubsystem::Deinitialize()
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->GetTimerManager().ClearTimer(EvaluationTimerHandle);
	}

	Super::Deinitialize();
}

void UPW_WildPalSpawnSubsystem::RegisterSpawner(APW_WildPalSpawner* Spawner)
{
	if (CanRunAuthoritySpawn() && IsValid(Spawner))
	{
		RegisteredSpawners.AddUnique(Spawner);
	}
}

void UPW_WildPalSpawnSubsystem::UnregisterSpawner(APW_WildPalSpawner* Spawner)
{
	RegisteredSpawners.Remove(Spawner);
}

bool UPW_WildPalSpawnSubsystem::TryReserveGlobalPal()
{
	if (!CanRunAuthoritySpawn() || ActivePalCount >= MaxActivePalCount)
	{
		return false;
	}

	++ActivePalCount;
	return true;
}

void UPW_WildPalSpawnSubsystem::ReleaseGlobalPal()
{
	ActivePalCount = FMath::Max(0, ActivePalCount - 1);
}

bool UPW_WildPalSpawnSubsystem::IsAnyPlayerWithin(const FVector& Origin, float Radius) const
{
	TArray<FVector> PlayerLocations;
	GatherPlayerLocations(PlayerLocations);

	const float RadiusSquared = FMath::Square(Radius);
	for (const FVector& PlayerLocation : PlayerLocations)
	{
		if (FVector::DistSquared(Origin, PlayerLocation) <= RadiusSquared)
		{
			return true;
		}
	}

	return false;
}

bool UPW_WildPalSpawnSubsystem::AreAllPlayersOutside(const FVector& Origin, float Radius) const
{
	TArray<FVector> PlayerLocations;
	GatherPlayerLocations(PlayerLocations);

	if (PlayerLocations.IsEmpty())
	{
		return true;
	}

	const float RadiusSquared = FMath::Square(Radius);
	for (const FVector& PlayerLocation : PlayerLocations)
	{
		if (FVector::DistSquared(Origin, PlayerLocation) <= RadiusSquared)
		{
			return false;
		}
	}

	return true;
}

int32 UPW_WildPalSpawnSubsystem::GetActivePalCount() const
{
	return ActivePalCount;
}

int32 UPW_WildPalSpawnSubsystem::GetMaxActivePalCount() const
{
	return MaxActivePalCount;
}

bool UPW_WildPalSpawnSubsystem::CanRunAuthoritySpawn() const
{
	const UWorld* World = GetWorld();
	return World != nullptr && World->GetAuthGameMode() != nullptr;
}

void UPW_WildPalSpawnSubsystem::EvaluateSpawners()
{
	if (!CanRunAuthoritySpawn())
	{
		return;
	}

	RegisteredSpawners.RemoveAll([](const TObjectPtr<APW_WildPalSpawner>& Spawner)
	{
		return !IsValid(Spawner);
	});

	for (APW_WildPalSpawner* Spawner : RegisteredSpawners)
	{
		if (IsValid(Spawner))
		{
			Spawner->EvaluateSpawner(*this);
		}
	}
}

void UPW_WildPalSpawnSubsystem::GatherPlayerLocations(TArray<FVector>& OutPlayerLocations) const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		const APawn* Pawn = PlayerController != nullptr ? PlayerController->GetPawn() : nullptr;
		if (Pawn != nullptr)
		{
			OutPlayerLocations.Add(Pawn->GetActorLocation());
		}
	}
}
