#include "Map/PW_MapExplorerComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Map/PW_MapSubsystem.h"
#include "TimerManager.h"

UPW_MapExplorerComponent::UPW_MapExplorerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPW_MapExplorerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bApplyMapSettingsOnBeginPlay)
	{
		ConfigureMapSubsystem();
	}

	if (bAutoReveal)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				RevealTimerHandle,
				this,
				&UPW_MapExplorerComponent::RevealOwnerLocation,
				RevealUpdateIntervalSeconds,
				true);
		}

		RevealOwnerLocation();
	}
}

void UPW_MapExplorerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UPW_MapExplorerComponent::ConfigureMapSubsystem()
{
	UPW_MapSubsystem* MapSubsystem = GetMapSubsystem();
	if (MapSubsystem == nullptr)
	{
		return;
	}

	MapSubsystem->SetMapBounds(WorldMin, WorldMax);
	MapSubsystem->SetExplorationGridSize(GridWidth, GridHeight);
	MapSubsystem->SetRevealRadius(RevealRadius);
}

void UPW_MapExplorerComponent::RevealOwnerLocation()
{
	UPW_MapSubsystem* MapSubsystem = GetMapSubsystem();
	const AActor* ExplorationActor = GetExplorationActor();
	if (MapSubsystem == nullptr || ExplorationActor == nullptr)
	{
		return;
	}

	MapSubsystem->RevealAroundLocation(GetResolvedPlayerId(), ExplorationActor->GetActorLocation());
}

bool UPW_MapExplorerComponent::GetOwnerMapUV(FVector2D& OutMapUV) const
{
	const UPW_MapSubsystem* MapSubsystem = GetMapSubsystem();
	const AActor* ExplorationActor = GetExplorationActor();
	if (MapSubsystem == nullptr || ExplorationActor == nullptr)
	{
		OutMapUV = FVector2D::ZeroVector;
		return false;
	}

	OutMapUV = MapSubsystem->WorldLocationToMapUV(ExplorationActor->GetActorLocation());
	return true;
}

void UPW_MapExplorerComponent::GetVisitedCellIndices(TArray<int32>& OutVisitedCellIndices) const
{
	const UPW_MapSubsystem* MapSubsystem = GetMapSubsystem();
	if (MapSubsystem == nullptr)
	{
		OutVisitedCellIndices.Reset();
		return;
	}

	MapSubsystem->GetVisitedCellIndices(GetResolvedPlayerId(), OutVisitedCellIndices);
}

FString UPW_MapExplorerComponent::GetResolvedPlayerId() const
{
	const AActor* Owner = GetOwner();
	if (bUseOwnerNameAsPlayerId && Owner != nullptr)
	{
		return Owner->GetName();
	}

	return PlayerId.IsEmpty() ? TEXT("LocalPlayer") : PlayerId;
}

FPW_MapExplorationSaveData UPW_MapExplorerComponent::MakeExplorationSaveData() const
{
	const UPW_MapSubsystem* MapSubsystem = GetMapSubsystem();
	if (MapSubsystem == nullptr)
	{
		FPW_MapExplorationSaveData EmptySaveData;
		EmptySaveData.PlayerId = GetResolvedPlayerId();
		return EmptySaveData;
	}

	return MapSubsystem->MakeExplorationSaveData(GetResolvedPlayerId());
}

bool UPW_MapExplorerComponent::ApplyExplorationSaveData(const FPW_MapExplorationSaveData& SaveData)
{
	UPW_MapSubsystem* MapSubsystem = GetMapSubsystem();
	if (MapSubsystem == nullptr)
	{
		return false;
	}

	return MapSubsystem->ApplyExplorationSaveData(SaveData);
}

UPW_MapSubsystem* UPW_MapExplorerComponent::GetMapSubsystem() const
{
	UWorld* World = GetWorld();
	return World != nullptr ? World->GetSubsystem<UPW_MapSubsystem>() : nullptr;
}

AActor* UPW_MapExplorerComponent::GetExplorationActor() const
{
	AActor* Owner = GetOwner();
	if (APlayerController* PlayerController = Cast<APlayerController>(Owner))
	{
		return PlayerController->GetPawn();
	}

	return Owner;
}
