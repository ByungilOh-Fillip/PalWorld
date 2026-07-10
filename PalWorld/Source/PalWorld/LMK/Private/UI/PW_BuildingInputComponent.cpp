#include "UI/PW_BuildingInputComponent.h"

#include "Base/PW_PlayerBuildingPlacementComponent.h"
#include "Base/PW_PlayerBasePlacementComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "TimerManager.h"
#include "UI/PW_BuildingRadialMenuWidget.h"

UPW_BuildingInputComponent::UPW_BuildingInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UPW_BuildingInputComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoBindInput)
	{
		TryBindInputKeys();
	}
}

void UPW_BuildingInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InputBindingRetryTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UPW_BuildingInputComponent::ToggleBuildingMenu()
{
	if (LastToggleFrame == GFrameCounter)
	{
		UE_LOG(LogTemp, Verbose, TEXT("PW_BuildingInputComponent ignored duplicate building menu toggle in the same frame."));
		return;
	}

	LastToggleFrame = GFrameCounter;
	const bool bTargetVisible = !IsBuildingMenuVisible();
	UE_LOG(LogTemp, Log, TEXT("PW_BuildingInputComponent toggle building menu: %s."), bTargetVisible ? TEXT("show") : TEXT("hide"));
	SetBuildingMenuVisible(bTargetVisible);
}

void UPW_BuildingInputComponent::ToggleDismantleMode()
{
	UPW_PlayerBuildingPlacementComponent* PlacementComponent = GetPlacementComponent();
	if (PlacementComponent == nullptr)
	{
		return;
	}

	if (PlacementComponent->IsPlacementModeActive() || PlacementComponent->IsDismantleModeActive() || IsBuildingMenuVisible())
	{
		PlacementComponent->SetDismantleModeActive(!PlacementComponent->IsDismantleModeActive());
		if (IsBuildingMenuVisible())
		{
			SetBuildingMenuVisible(false);
		}
	}
}

void UPW_BuildingInputComponent::SetBuildingMenuVisible(bool bVisible)
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController == nullptr || !PlayerController->IsLocalController())
	{
		return;
	}

	if (UPW_PlayerBuildingPlacementComponent* PlacementComponent = GetPlacementComponent())
	{
		PlacementComponent->CancelPlacement();
	}

	UPW_BuildingRadialMenuWidget* ActiveMenu = GetOrCreateBuildingRadialMenu();
	if (ActiveMenu)
	{
		ActiveMenu->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	PlayerController->bShowMouseCursor = bVisible;
	if (bVisible)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
		return;
	}

	PlayerController->SetInputMode(FInputModeGameOnly());
}

bool UPW_BuildingInputComponent::HandlePrimaryActionPressed()
{
	UPW_PlayerBuildingPlacementComponent* PlacementComponent = GetPlacementComponent();
	if (PlacementComponent == nullptr)
	{
		return false;
	}

	if (PlacementComponent->IsDismantleModeActive())
	{
		PlacementComponent->ConfirmDismantle();
		return true;
	}

	if (!PlacementComponent->IsPlacementModeActive())
	{
		return false;
	}

	PlacementComponent->ConfirmPlacement();
	return true;
}

bool UPW_BuildingInputComponent::HandleSecondaryActionPressed()
{
	UPW_PlayerBuildingPlacementComponent* PlacementComponent = GetPlacementComponent();
	if (PlacementComponent == nullptr)
	{
		return false;
	}

	if (PlacementComponent->IsDismantleModeActive())
	{
		PlacementComponent->SetDismantleModeActive(false);
		return true;
	}

	if (!PlacementComponent->IsPlacementModeActive())
	{
		return false;
	}

	PlacementComponent->CancelPlacement();
	return true;
}

bool UPW_BuildingInputComponent::HandleWheelNext()
{
	UPW_PlayerBuildingPlacementComponent* PlacementComponent = GetPlacementComponent();
	if (PlacementComponent == nullptr || !PlacementComponent->IsPlacementModeActive())
	{
		return false;
	}

	PlacementComponent->RotatePreviewByWheel(1.0f);
	return true;
}

bool UPW_BuildingInputComponent::HandleWheelPrevious()
{
	UPW_PlayerBuildingPlacementComponent* PlacementComponent = GetPlacementComponent();
	if (PlacementComponent == nullptr || !PlacementComponent->IsPlacementModeActive())
	{
		return false;
	}

	PlacementComponent->RotatePreviewByWheel(-1.0f);
	return true;
}

bool UPW_BuildingInputComponent::HandleLookInput(const FVector2D& LookVector)
{
	UPW_PlayerBuildingPlacementComponent* PlacementComponent = GetPlacementComponent();
	return PlacementComponent != nullptr && PlacementComponent->TryConsumeLookInput(LookVector);
}

bool UPW_BuildingInputComponent::IsBuildingPlacementActive() const
{
	const UPW_PlayerBuildingPlacementComponent* PlacementComponent = GetPlacementComponent();
	return PlacementComponent != nullptr && PlacementComponent->IsPlacementModeActive();
}

bool UPW_BuildingInputComponent::IsBuildingMenuVisible() const
{
	return BuildingRadialMenuWidget != nullptr && BuildingRadialMenuWidget->IsVisible();
}

UPW_PlayerBuildingPlacementComponent* UPW_BuildingInputComponent::GetPlacementComponent() const
{
	AActor* Owner = GetOwner();
	if (APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		return OwnerPawn->FindComponentByClass<UPW_PlayerBuildingPlacementComponent>();
	}

	APlayerController* PlayerController = Cast<APlayerController>(Owner);
	APawn* ControlledPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (ControlledPawn)
	{
		return ControlledPawn->FindComponentByClass<UPW_PlayerBuildingPlacementComponent>();
	}

	return Owner ? Owner->FindComponentByClass<UPW_PlayerBuildingPlacementComponent>() : nullptr;
}

UPW_PlayerBasePlacementComponent* UPW_BuildingInputComponent::GetBasePlacementComponent() const
{
	AActor* Owner = GetOwner();
	if (APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		return OwnerPawn->FindComponentByClass<UPW_PlayerBasePlacementComponent>();
	}

	APlayerController* PlayerController = Cast<APlayerController>(Owner);
	APawn* ControlledPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (ControlledPawn)
	{
		return ControlledPawn->FindComponentByClass<UPW_PlayerBasePlacementComponent>();
	}

	return Owner ? Owner->FindComponentByClass<UPW_PlayerBasePlacementComponent>() : nullptr;
}

APlayerController* UPW_BuildingInputComponent::GetOwningPlayerController() const
{
	AActor* Owner = GetOwner();
	if (APlayerController* PlayerController = Cast<APlayerController>(Owner))
	{
		return PlayerController;
	}

	const APawn* OwnerPawn = Cast<APawn>(Owner);
	return OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
}

UPW_BuildingRadialMenuWidget* UPW_BuildingInputComponent::GetOrCreateBuildingRadialMenu()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController == nullptr)
	{
		return nullptr;
	}

	if (!BuildingRadialMenuWidget)
	{
		TSubclassOf<UPW_BuildingRadialMenuWidget> MenuClass = BuildingRadialMenuWidgetClass;
		if (!MenuClass)
		{
			MenuClass = UPW_BuildingRadialMenuWidget::StaticClass();
		}

		BuildingRadialMenuWidget = CreateWidget<UPW_BuildingRadialMenuWidget>(PlayerController, MenuClass);
		if (BuildingRadialMenuWidget)
		{
			UE_LOG(LogTemp, Log, TEXT("PW_BuildingInputComponent created building menu widget: %s."), *GetNameSafe(BuildingRadialMenuWidget));
			BuildingRadialMenuWidget->AddToViewport(BuildingMenuZOrder);
			BuildingRadialMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PW_BuildingInputComponent failed to create building menu widget."));
		}
	}

	if (BuildingRadialMenuWidget)
	{
		BuildingRadialMenuWidget->InitializeWithBuildingPlacementComponent(GetPlacementComponent());
	}

	return BuildingRadialMenuWidget;
}

void UPW_BuildingInputComponent::TryBindInputKeys()
{
	BindInputKeys();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	APlayerController* PlayerController = GetOwningPlayerController();
	if (bInputBound || (PlayerController != nullptr && !PlayerController->IsLocalController()))
	{
		World->GetTimerManager().ClearTimer(InputBindingRetryTimerHandle);
		return;
	}

	if (!World->GetTimerManager().IsTimerActive(InputBindingRetryTimerHandle))
	{
		World->GetTimerManager().SetTimer(
			InputBindingRetryTimerHandle,
			this,
			&UPW_BuildingInputComponent::TryBindInputKeys,
			0.1f,
			true);
	}
}

void UPW_BuildingInputComponent::BindInputKeys()
{
	if (bInputBound)
	{
		return;
	}

	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController == nullptr || !PlayerController->IsLocalController() || PlayerController->InputComponent == nullptr)
	{
		return;
	}

	FInputKeyBinding& BuildingMenuBinding = PlayerController->InputComponent->BindKey(EKeys::B, IE_Pressed, this, &UPW_BuildingInputComponent::ToggleBuildingMenu);
	BuildingMenuBinding.bConsumeInput = bConsumeAutoBoundInput;

	FInputKeyBinding& DismantleBinding = PlayerController->InputComponent->BindKey(EKeys::C, IE_Pressed, this, &UPW_BuildingInputComponent::ToggleDismantleMode);
	DismantleBinding.bConsumeInput = bConsumeAutoBoundInput;

	FInputKeyBinding& BasePlacementBinding = PlayerController->InputComponent->BindKey(EKeys::G, IE_Pressed, this, &UPW_BuildingInputComponent::HandleAutoBasePlacementPressed);
	BasePlacementBinding.bConsumeInput = bConsumeAutoBoundInput;

	FInputKeyBinding& PrimaryBinding = PlayerController->InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &UPW_BuildingInputComponent::HandleAutoPrimaryPressed);
	PrimaryBinding.bConsumeInput = bConsumeAutoBoundInput;

	FInputKeyBinding& SecondaryBinding = PlayerController->InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &UPW_BuildingInputComponent::HandleAutoSecondaryPressed);
	SecondaryBinding.bConsumeInput = bConsumeAutoBoundInput;

	FInputKeyBinding& WheelUpBinding = PlayerController->InputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &UPW_BuildingInputComponent::HandleAutoWheelNext);
	WheelUpBinding.bConsumeInput = bConsumeAutoBoundInput;

	FInputKeyBinding& WheelDownBinding = PlayerController->InputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &UPW_BuildingInputComponent::HandleAutoWheelPrevious);
	WheelDownBinding.bConsumeInput = bConsumeAutoBoundInput;

	bInputBound = true;
	UE_LOG(LogTemp, Log, TEXT("PW_BuildingInputComponent bound building input keys on %s."), *GetNameSafe(PlayerController));
}

void UPW_BuildingInputComponent::HandleAutoPrimaryPressed()
{
	HandlePrimaryActionPressed();
}

void UPW_BuildingInputComponent::HandleAutoSecondaryPressed()
{
	HandleSecondaryActionPressed();
}

void UPW_BuildingInputComponent::HandleAutoWheelNext()
{
	HandleWheelNext();
}

void UPW_BuildingInputComponent::HandleAutoWheelPrevious()
{
	HandleWheelPrevious();
}

void UPW_BuildingInputComponent::HandleAutoBasePlacementPressed()
{
	UPW_PlayerBasePlacementComponent* BasePlacementComponent = GetBasePlacementComponent();
	if (BasePlacementComponent == nullptr)
	{
		return;
	}

	if (!BasePlacementComponent->IsPlacementModeActive())
	{
		if (!IsBuildingMenuVisible() && !IsBuildingPlacementActive())
		{
			return;
		}

		if (UPW_PlayerBuildingPlacementComponent* PlacementComponent = GetPlacementComponent())
		{
			PlacementComponent->CancelPlacement();
		}

		if (IsBuildingMenuVisible())
		{
			SetBuildingMenuVisible(false);
		}
	}

	BasePlacementComponent->TogglePlacementMode();
}
