#include "Map/PW_MapSubsystem.h"

#include "Base/PW_BaseCampActor.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Map/PW_TeleportPointActor.h"
#include "TimerManager.h"

void UPW_MapSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (InWorld.IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	InWorld.GetTimerManager().SetTimer(
		ExplorationUpdateTimerHandle,
		this,
		&UPW_MapSubsystem::UpdateLocalPlayerExploration,
		ExplorationUpdateIntervalSeconds,
		true);

	UpdateLocalPlayerExploration();
}

void UPW_MapSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExplorationUpdateTimerHandle);
	}

	Super::Deinitialize();
}

void UPW_MapSubsystem::SetMapBounds(const FVector2D& NewWorldMin, const FVector2D& NewWorldMax)
{
	WorldMin = NewWorldMin;
	WorldMax = NewWorldMax;
}

void UPW_MapSubsystem::SetExplorationGridSize(int32 NewGridWidth, int32 NewGridHeight)
{
	GridWidth = FMath::Max(1, NewGridWidth);
	GridHeight = FMath::Max(1, NewGridHeight);
}

void UPW_MapSubsystem::SetRevealRadius(float NewRevealRadius)
{
	RevealRadius = FMath::Max(0.0f, NewRevealRadius);
}

FVector2D UPW_MapSubsystem::WorldLocationToMapUV(const FVector& WorldLocation) const
{
	if (!IsMapConfigured())
	{
		return FVector2D::ZeroVector;
	}

	const float U = (WorldLocation.X - WorldMin.X) / (WorldMax.X - WorldMin.X);
	const float V = (WorldLocation.Y - WorldMin.Y) / (WorldMax.Y - WorldMin.Y);
	return FVector2D(FMath::Clamp(U, 0.0f, 1.0f), FMath::Clamp(V, 0.0f, 1.0f));
}

void UPW_MapSubsystem::RevealAroundLocation(const FString& PlayerId, const FVector& WorldLocation)
{
	if (PlayerId.IsEmpty() || !IsMapConfigured())
	{
		return;
	}

	const FVector2D MapUV = WorldLocationToMapUV(WorldLocation);
	const int32 CenterCellX = FMath::Clamp(FMath::FloorToInt(MapUV.X * GridWidth), 0, GridWidth - 1);
	const int32 CenterCellY = FMath::Clamp(FMath::FloorToInt(MapUV.Y * GridHeight), 0, GridHeight - 1);

	const float CellWorldWidth = (WorldMax.X - WorldMin.X) / GridWidth;
	const float CellWorldHeight = (WorldMax.Y - WorldMin.Y) / GridHeight;
	const float CellWorldSize = FMath::Max(CellWorldWidth, CellWorldHeight);
	const int32 RevealCellRadius = FMath::Max(0, FMath::CeilToInt(RevealRadius / FMath::Max(CellWorldSize, 1.0f)));

	for (int32 CellY = CenterCellY - RevealCellRadius; CellY <= CenterCellY + RevealCellRadius; ++CellY)
	{
		for (int32 CellX = CenterCellX - RevealCellRadius; CellX <= CenterCellX + RevealCellRadius; ++CellX)
		{
			const FVector2D CellCenterWorld(
				WorldMin.X + (CellX + 0.5f) * CellWorldWidth,
				WorldMin.Y + (CellY + 0.5f) * CellWorldHeight);
			const FVector2D PlayerWorld2D(WorldLocation.X, WorldLocation.Y);
			if (FVector2D::Distance(CellCenterWorld, PlayerWorld2D) <= RevealRadius)
			{
				AddVisitedCell(PlayerId, CellX, CellY);
			}
		}
	}
}

void UPW_MapSubsystem::GetVisitedCellIndices(const FString& PlayerId, TArray<int32>& OutVisitedCellIndices) const
{
	OutVisitedCellIndices.Reset();

	const FPW_MapExplorationRuntimeState* RuntimeState = ExplorationByPlayerId.Find(PlayerId);
	if (RuntimeState == nullptr)
	{
		return;
	}

	for (const int32 CellIndex : RuntimeState->VisitedCellIndices)
	{
		OutVisitedCellIndices.Add(CellIndex);
	}
	OutVisitedCellIndices.Sort();
}

void UPW_MapSubsystem::GetMapMarkers(TArray<FPW_MapMarker>& OutMarkers) const
{
	GetMapMarkersForPlayerController(nullptr, OutMarkers);
}

void UPW_MapSubsystem::GetMapMarkersForPlayerController(const APlayerController* PlayerController, TArray<FPW_MapMarker>& OutMarkers) const
{
	GetMapMarkersForPlayer(GetLocalPlayerId(), PlayerController, OutMarkers);
}

void UPW_MapSubsystem::GetMapMarkersForPlayer(const FString& PlayerId, const APlayerController* PlayerController, TArray<FPW_MapMarker>& OutMarkers) const
{
	OutMarkers.Reset();
	AddTeleportMarkers(PlayerId, OutMarkers);
	AddBaseCampMarkers(OutMarkers, PlayerController);
}

bool UPW_MapSubsystem::GetTeleportDestination(EPW_MapMarkerType MarkerType, FName MarkerId, FVector& OutWorldLocation) const
{
	return GetTeleportDestinationForPlayerController(nullptr, MarkerType, MarkerId, OutWorldLocation);
}

bool UPW_MapSubsystem::GetTeleportDestinationForPlayerController(const APlayerController* PlayerController, EPW_MapMarkerType MarkerType, FName MarkerId, FVector& OutWorldLocation) const
{
	UWorld* World = GetWorld();
	if (World == nullptr || MarkerId.IsNone())
	{
		return false;
	}

	if (MarkerType == EPW_MapMarkerType::TeleportPoint)
	{
		for (TActorIterator<APW_TeleportPointActor> It(World); It; ++It)
		{
			const APW_TeleportPointActor* TeleportPoint = *It;
			if (!IsValid(TeleportPoint))
			{
				continue;
			}

			const FName ResolvedTeleportPointId = TeleportPoint->GetTeleportPointId().IsNone()
				? FName(*TeleportPoint->GetName())
				: TeleportPoint->GetTeleportPointId();
			if (ResolvedTeleportPointId == MarkerId
				&& TeleportPoint->IsDiscovered()
				&& TeleportPoint->CanTeleport())
			{
				OutWorldLocation = TeleportPoint->GetTeleportArrivalLocation();
				return true;
			}
		}

		return false;
	}

	if (MarkerType == EPW_MapMarkerType::BaseCamp)
	{
		for (TActorIterator<APW_BaseCampActor> It(World); It; ++It)
		{
			const APW_BaseCampActor* BaseCamp = *It;
			if (IsValid(BaseCamp)
				&& FName(*BaseCamp->GetBaseCampId().Value.ToString()) == MarkerId
				&& CanPlayerUseBaseCamp(PlayerController, BaseCamp))
			{
				OutWorldLocation = BaseCamp->GetActorLocation();
				return true;
			}
		}
	}

	return false;
}

bool UPW_MapSubsystem::GetLocalPlayerMapUV(FVector2D& OutPlayerMapUV) const
{
	const APawn* LocalPawn = GetLocalPlayerPawn();
	if (LocalPawn == nullptr)
	{
		OutPlayerMapUV = FVector2D::ZeroVector;
		return false;
	}

	OutPlayerMapUV = WorldLocationToMapUV(LocalPawn->GetActorLocation());
	return true;
}

float UPW_MapSubsystem::GetRevealRadiusUV() const
{
	if (!IsMapConfigured())
	{
		return 0.0f;
	}

	const float WorldWidth = WorldMax.X - WorldMin.X;
	const float WorldHeight = WorldMax.Y - WorldMin.Y;
	const float WorldSize = FMath::Max(WorldWidth, WorldHeight);
	return RevealRadius / FMath::Max(WorldSize, 1.0f);
}

FPW_MapExplorationSaveData UPW_MapSubsystem::MakeExplorationSaveData(const FString& PlayerId) const
{
	FPW_MapExplorationSaveData SaveData;
	SaveData.PlayerId = PlayerId;
	SaveData.MapId = MapId;
	SaveData.GridWidth = GridWidth;
	SaveData.GridHeight = GridHeight;
	GetVisitedCellIndices(PlayerId, SaveData.VisitedCellIndices);
	return SaveData;
}

bool UPW_MapSubsystem::ApplyExplorationSaveData(const FPW_MapExplorationSaveData& SaveData)
{
	if (SaveData.PlayerId.IsEmpty() || !SaveData.IsCompatible(MapId, GridWidth, GridHeight))
	{
		return false;
	}

	FPW_MapExplorationRuntimeState& RuntimeState = ExplorationByPlayerId.FindOrAdd(SaveData.PlayerId);
	RuntimeState.VisitedCellIndices.Reset();

	for (const int32 CellIndex : SaveData.VisitedCellIndices)
	{
		if (IsValidCellIndex(CellIndex))
		{
			RuntimeState.VisitedCellIndices.Add(CellIndex);
		}
	}

	return true;
}

bool UPW_MapSubsystem::IsMapConfigured() const
{
	return GridWidth > 0 && GridHeight > 0 && WorldMax.X > WorldMin.X && WorldMax.Y > WorldMin.Y;
}

bool UPW_MapSubsystem::IsValidCellIndex(int32 CellIndex) const
{
	return CellIndex >= 0 && CellIndex < GridWidth * GridHeight;
}

int32 UPW_MapSubsystem::GetCellIndex(int32 CellX, int32 CellY) const
{
	return CellY * GridWidth + CellX;
}

FString UPW_MapSubsystem::GetLocalPlayerId() const
{
	return TEXT("LocalPlayer");
}

APawn* UPW_MapSubsystem::GetLocalPlayerPawn() const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (PlayerController != nullptr && PlayerController->IsLocalController())
		{
			return PlayerController->GetPawn();
		}
	}

	return nullptr;
}

void UPW_MapSubsystem::UpdateLocalPlayerExploration()
{
	const APawn* LocalPawn = GetLocalPlayerPawn();
	if (LocalPawn == nullptr)
	{
		return;
	}

	RevealAroundLocation(GetLocalPlayerId(), LocalPawn->GetActorLocation());
}

void UPW_MapSubsystem::AddVisitedCell(const FString& PlayerId, int32 CellX, int32 CellY)
{
	if (CellX < 0 || CellX >= GridWidth || CellY < 0 || CellY >= GridHeight)
	{
		return;
	}

	const int32 CellIndex = GetCellIndex(CellX, CellY);
	FPW_MapExplorationRuntimeState& RuntimeState = ExplorationByPlayerId.FindOrAdd(PlayerId);
	RuntimeState.VisitedCellIndices.Add(CellIndex);
}

bool UPW_MapSubsystem::IsWorldLocationVisited(const FString& PlayerId, const FVector& WorldLocation) const
{
	if (PlayerId.IsEmpty() || !IsMapConfigured())
	{
		return false;
	}

	const FPW_MapExplorationRuntimeState* RuntimeState = ExplorationByPlayerId.Find(PlayerId);
	if (RuntimeState == nullptr)
	{
		return false;
	}

	const FVector2D MapUV = WorldLocationToMapUV(WorldLocation);
	const int32 CellX = FMath::Clamp(FMath::FloorToInt(MapUV.X * GridWidth), 0, GridWidth - 1);
	const int32 CellY = FMath::Clamp(FMath::FloorToInt(MapUV.Y * GridHeight), 0, GridHeight - 1);
	return RuntimeState->VisitedCellIndices.Contains(GetCellIndex(CellX, CellY));
}

void UPW_MapSubsystem::AddTeleportMarkers(const FString& PlayerId, TArray<FPW_MapMarker>& OutMarkers) const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (TActorIterator<APW_TeleportPointActor> It(World); It; ++It)
	{
		const APW_TeleportPointActor* TeleportPoint = *It;
		if (!IsValid(TeleportPoint))
		{
			continue;
		}

		const bool bIsDiscovered = TeleportPoint->IsDiscovered();
		const bool bIsMapVisited = IsWorldLocationVisited(PlayerId, TeleportPoint->GetActorLocation());
		if (!bIsDiscovered && !bIsMapVisited)
		{
			continue;
		}

		const FName MarkerId = TeleportPoint->GetTeleportPointId().IsNone()
			? FName(*TeleportPoint->GetName())
			: TeleportPoint->GetTeleportPointId();

		FPW_MapMarker Marker;
		Marker.MarkerType = EPW_MapMarkerType::TeleportPoint;
		Marker.MarkerId = MarkerId;
		Marker.DisplayName = TeleportPoint->GetDisplayName();
		Marker.WorldLocation = TeleportPoint->GetActorLocation();
		Marker.MapUV = WorldLocationToMapUV(Marker.WorldLocation);
		Marker.bDiscovered = bIsDiscovered;
		Marker.bCanTeleport = bIsDiscovered && TeleportPoint->CanTeleport();
		OutMarkers.Add(Marker);
	}
}

void UPW_MapSubsystem::AddBaseCampMarkers(TArray<FPW_MapMarker>& OutMarkers, const APlayerController* PlayerController) const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (TActorIterator<APW_BaseCampActor> It(World); It; ++It)
	{
		const APW_BaseCampActor* BaseCamp = *It;
		if (!IsValid(BaseCamp))
		{
			continue;
		}

		FPW_MapMarker Marker;
		Marker.MarkerType = EPW_MapMarkerType::BaseCamp;
		Marker.MarkerId = FName(*BaseCamp->GetBaseCampId().Value.ToString());
		Marker.DisplayName = FText::FromString(BaseCamp->GetName());
		Marker.WorldLocation = BaseCamp->GetActorLocation();
		Marker.MapUV = WorldLocationToMapUV(Marker.WorldLocation);
		Marker.bDiscovered = true;
		Marker.bCanTeleport = CanPlayerUseBaseCamp(PlayerController, BaseCamp);
		OutMarkers.Add(Marker);
	}
}

FPW_BaseOwnerId UPW_MapSubsystem::MakeBaseOwnerIdFromPlayerController(const APlayerController* PlayerController) const
{
	FPW_BaseOwnerId OwnerId;
	OwnerId.OwnerType = EPW_BaseOwnerType::Player;

	const APawn* Pawn = PlayerController != nullptr ? PlayerController->GetPawn() : nullptr;
	const APlayerState* PlayerState = Pawn != nullptr ? Pawn->GetPlayerState() : nullptr;
	if (PlayerState != nullptr && PlayerState->GetPlayerId() != INDEX_NONE)
	{
		OwnerId.OwnerId = FString::FromInt(PlayerState->GetPlayerId());
	}
	else if (Pawn != nullptr)
	{
		OwnerId.OwnerId = GetNameSafe(Pawn);
	}

	return OwnerId;
}

bool UPW_MapSubsystem::CanPlayerUseBaseCamp(const APlayerController* PlayerController, const APW_BaseCampActor* BaseCamp) const
{
	if (PlayerController == nullptr || BaseCamp == nullptr)
	{
		return false;
	}

	const FPW_BaseOwnerId RequesterOwnerId = MakeBaseOwnerIdFromPlayerController(PlayerController);
	return RequesterOwnerId.IsValid() && BaseCamp->GetBaseOwnerId() == RequesterOwnerId;
}
