#include "UI/PW_WorldMapControllerComponent.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Map/PW_MapSubsystem.h"
#include "Map/PW_TeleportPointActor.h"
#include "TimerManager.h"
#include "UI/PW_WorldMapWidget.h"

UPW_WorldMapControllerComponent::UPW_WorldMapControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPW_WorldMapControllerComponent::BeginPlay()
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
				&UPW_WorldMapControllerComponent::RevealControlledPawnLocation,
				RevealUpdateIntervalSeconds,
				true);
		}

		RevealControlledPawnLocation();
	}
}

void UPW_WorldMapControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
	}

	HideWorldMap();
	Super::EndPlay(EndPlayReason);
}

void UPW_WorldMapControllerComponent::ShowWorldMap()
{
	if (!bTeleportSelectionMode)
	{
		ActiveTeleportSource = nullptr;
	}

	ShowWorldMapInternal(false);
}

void UPW_WorldMapControllerComponent::ShowTeleportMap()
{
	ShowWorldMapInternal(true);
}

void UPW_WorldMapControllerComponent::ShowWorldMapInternal(bool bEnableTeleportSelection)
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController == nullptr || WorldMapWidgetClass == nullptr)
	{
		return;
	}

	if (IsWorldMapVisible())
	{
		bTeleportSelectionMode = bEnableTeleportSelection;
		ConfigureWorldMapWidget(WorldMapWidgetInstance);
		return;
	}

	bTeleportSelectionMode = bEnableTeleportSelection;
	WorldMapWidgetInstance = CreateWidget<UPW_WorldMapWidget>(PlayerController, WorldMapWidgetClass);
	if (WorldMapWidgetInstance == nullptr)
	{
		return;
	}

	ConfigureMapSubsystem();
	RevealControlledPawnLocation();
	ConfigureWorldMapWidget(WorldMapWidgetInstance);
	WorldMapWidgetInstance->AddToViewport(WorldMapZOrder);
	ApplyShowInputMode();
}

void UPW_WorldMapControllerComponent::HideWorldMap()
{
	if (WorldMapWidgetInstance != nullptr)
	{
		WorldMapWidgetInstance->RemoveFromParent();
		WorldMapWidgetInstance = nullptr;
	}

	bTeleportSelectionMode = false;
	ApplyHideInputMode();
}

void UPW_WorldMapControllerComponent::ToggleWorldMap()
{
	if (IsWorldMapVisible())
	{
		HideWorldMap();
		return;
	}

	ShowWorldMap();
}

bool UPW_WorldMapControllerComponent::IsWorldMapVisible() const
{
	return WorldMapWidgetInstance != nullptr && WorldMapWidgetInstance->IsInViewport();
}

void UPW_WorldMapControllerComponent::ConfigureMapSubsystem()
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

void UPW_WorldMapControllerComponent::RevealControlledPawnLocation()
{
	UPW_MapSubsystem* MapSubsystem = GetMapSubsystem();
	const APawn* Pawn = GetControlledPawn();
	if (MapSubsystem == nullptr || Pawn == nullptr)
	{
		return;
	}

	MapSubsystem->RevealAroundLocation(GetResolvedPlayerId(), Pawn->GetActorLocation());
}

void UPW_WorldMapControllerComponent::RequestTeleportToMarker(EPW_MapMarkerType MarkerType, FName MarkerId)
{
	if (MarkerId.IsNone())
	{
		return;
	}

	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController == nullptr || !PlayerController->IsLocalController())
	{
		return;
	}

	ServerRequestTeleportToMarker(MarkerType, MarkerId);
	HideWorldMap();
}

void UPW_WorldMapControllerComponent::SetActiveTeleportSource(APW_TeleportPointActor* TeleportSource)
{
	if (GetOwner() == nullptr || !GetOwner()->HasAuthority())
	{
		return;
	}

	ActiveTeleportSource = TeleportSource;
}

void UPW_WorldMapControllerComponent::ClientShowTeleportMap_Implementation()
{
	ShowTeleportMap();
}

FString UPW_WorldMapControllerComponent::GetResolvedPlayerId() const
{
	return PlayerId.IsEmpty() ? TEXT("LocalPlayer") : PlayerId;
}

FPW_MapExplorationSaveData UPW_WorldMapControllerComponent::MakeExplorationSaveData() const
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

bool UPW_WorldMapControllerComponent::ApplyExplorationSaveData(const FPW_MapExplorationSaveData& SaveData)
{
	UPW_MapSubsystem* MapSubsystem = GetMapSubsystem();
	if (MapSubsystem == nullptr)
	{
		return false;
	}

	return MapSubsystem->ApplyExplorationSaveData(SaveData);
}

APlayerController* UPW_WorldMapControllerComponent::GetOwningPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

APawn* UPW_WorldMapControllerComponent::GetControlledPawn() const
{
	const APlayerController* PlayerController = GetOwningPlayerController();
	return PlayerController != nullptr ? PlayerController->GetPawn() : nullptr;
}

UPW_MapSubsystem* UPW_WorldMapControllerComponent::GetMapSubsystem() const
{
	UWorld* World = GetWorld();
	return World != nullptr ? World->GetSubsystem<UPW_MapSubsystem>() : nullptr;
}

void UPW_WorldMapControllerComponent::ConfigureWorldMapWidget(UPW_WorldMapWidget* Widget) const
{
	if (Widget == nullptr)
	{
		return;
	}

	Widget->ConfigureMapWidgetWithSettings(
		GetResolvedPlayerId(),
		GetControlledPawn(),
		WorldMin,
		WorldMax,
		GridWidth,
		GridHeight,
		RevealRadius);
	Widget->SetTeleportSelectionEnabled(bTeleportSelectionMode);
}

void UPW_WorldMapControllerComponent::ApplyShowInputMode()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController == nullptr)
	{
		return;
	}

	bPreviousShowMouseCursor = PlayerController->bShowMouseCursor;
	bHasAppliedWorldMapInputMode = true;
	PlayerController->bShowMouseCursor = true;

	if (!bApplyGameAndUIInputMode)
	{
		return;
	}

	if (bUseUIOnlyInputModeWhenMapVisible)
	{
		FInputModeUIOnly InputMode;
		if (WorldMapWidgetInstance != nullptr)
		{
			InputMode.SetWidgetToFocus(WorldMapWidgetInstance->TakeWidget());
			WorldMapWidgetInstance->SetKeyboardFocus();
		}
		PlayerController->SetInputMode(InputMode);
		return;
	}

	FInputModeGameAndUI InputMode;
	if (WorldMapWidgetInstance != nullptr)
	{
		InputMode.SetWidgetToFocus(WorldMapWidgetInstance->TakeWidget());
		WorldMapWidgetInstance->SetKeyboardFocus();
	}
	InputMode.SetHideCursorDuringCapture(false);
	PlayerController->SetInputMode(InputMode);
}

void UPW_WorldMapControllerComponent::ApplyHideInputMode()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController == nullptr || !bHasAppliedWorldMapInputMode)
	{
		return;
	}

	PlayerController->bShowMouseCursor = bPreviousShowMouseCursor;
	bHasAppliedWorldMapInputMode = false;

	if (bRestoreGameOnlyInputModeOnHide)
	{
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
	}
}

bool UPW_WorldMapControllerComponent::CanUseActiveTeleportSource() const
{
	const APawn* Pawn = GetControlledPawn();
	return ActiveTeleportSource != nullptr
		&& IsValid(ActiveTeleportSource)
		&& ActiveTeleportSource->IsDiscovered()
		&& ActiveTeleportSource->CanTeleport()
		&& ActiveTeleportSource->CanUseAsTeleportSource(const_cast<APawn*>(Pawn));
}

void UPW_WorldMapControllerComponent::ServerRequestTeleportToMarker_Implementation(EPW_MapMarkerType MarkerType, FName MarkerId)
{
	UPW_MapSubsystem* MapSubsystem = GetMapSubsystem();
	APawn* Pawn = GetControlledPawn();
	APlayerController* PlayerController = GetOwningPlayerController();
	if (MapSubsystem == nullptr || Pawn == nullptr || PlayerController == nullptr || MarkerId.IsNone() || !CanUseActiveTeleportSource())
	{
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Red,
				TEXT("[Map] Server teleport rejected. Invalid controller, pawn, marker, or teleport source."));
		}
		return;
	}

	FVector Destination = FVector::ZeroVector;
	if (!MapSubsystem->GetTeleportDestinationForPlayerController(PlayerController, MarkerType, MarkerId, Destination))
	{
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Red,
				FString::Printf(TEXT("[Map] Server teleport rejected. Invalid destination: %s"), *MarkerId.ToString()));
		}
		return;
	}

	const FVector FinalDestination = MarkerType == EPW_MapMarkerType::TeleportPoint
		? Destination
		: Destination + TeleportArrivalOffset;
	if (!Pawn->TeleportTo(FinalDestination, Pawn->GetActorRotation()))
	{
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Red,
				FString::Printf(TEXT("[Map] Server teleport failed. Could not place pawn at destination: %s"), *MarkerId.ToString()));
		}
		return;
	}

	ActiveTeleportSource = nullptr;
}
