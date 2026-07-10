#include "Menu/PW_MenuPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "UI/PW_MainMenuWidget.h"

void APW_MenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ShowMainMenu();
}

void APW_MenuPlayerController::ShowMainMenu()
{
	if (!IsLocalController())
	{
		return;
	}

	if (MainMenuWidget == nullptr)
	{
		TSubclassOf<UPW_MainMenuWidget> WidgetClass = MainMenuWidgetClass;
		if (WidgetClass == nullptr)
		{
			WidgetClass = UPW_MainMenuWidget::StaticClass();
		}

		MainMenuWidget = CreateWidget<UPW_MainMenuWidget>(this, WidgetClass);
	}

	if (MainMenuWidget != nullptr && !MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->AddToViewport(MainMenuZOrder);
	}

	ApplyMenuInputMode();
}

void APW_MenuPlayerController::HideMainMenu()
{
	if (MainMenuWidget != nullptr)
	{
		MainMenuWidget->RemoveFromParent();
		MainMenuWidget = nullptr;
	}
}

void APW_MenuPlayerController::ApplyMenuInputMode()
{
	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	if (MainMenuWidget != nullptr)
	{
		InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
	}
	SetInputMode(InputMode);
}
