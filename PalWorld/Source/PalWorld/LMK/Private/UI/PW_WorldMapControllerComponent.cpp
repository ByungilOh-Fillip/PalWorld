#include "UI/PW_WorldMapControllerComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Map/PW_MapExplorerComponent.h"
#include "UI/PW_WorldMapWidget.h"

UPW_WorldMapControllerComponent::UPW_WorldMapControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPW_WorldMapControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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

APlayerController* UPW_WorldMapControllerComponent::GetOwningPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

void UPW_WorldMapControllerComponent::ConfigureWorldMapWidget(UPW_WorldMapWidget* Widget) const
{
	if (Widget == nullptr)
	{
		return;
	}

	APlayerController* PlayerController = GetOwningPlayerController();
	APawn* Pawn = PlayerController != nullptr ? PlayerController->GetPawn() : nullptr;
	const UPW_MapExplorerComponent* ExplorerComponent = Pawn != nullptr ? Pawn->FindComponentByClass<UPW_MapExplorerComponent>() : nullptr;
	if (ExplorerComponent == nullptr && PlayerController != nullptr)
	{
		ExplorerComponent = PlayerController->FindComponentByClass<UPW_MapExplorerComponent>();
	}

	if (ExplorerComponent == nullptr)
	{
		return;
	}

	Widget->ConfigureMapWidget(
		ExplorerComponent->GetResolvedPlayerId(),
		Pawn);
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
