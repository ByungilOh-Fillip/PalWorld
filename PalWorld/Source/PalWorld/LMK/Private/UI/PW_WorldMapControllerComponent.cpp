#include "UI/PW_WorldMapControllerComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Map/PW_MapSubsystem.h"
#include "TimerManager.h"
#include "UI/PW_WorldMapWidget.h"

UPW_WorldMapControllerComponent::UPW_WorldMapControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController == nullptr || WorldMapWidgetClass == nullptr)
	{
		return;
	}

	if (IsWorldMapVisible())
	{
		return;
	}

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

	FInputModeGameAndUI InputMode;
	if (WorldMapWidgetInstance != nullptr)
	{
		InputMode.SetWidgetToFocus(WorldMapWidgetInstance->TakeWidget());
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
