#include "Merchant/PW_MerchantPalCharacter.h"

#include "Merchant/PWPlayerTradeComponent.h"
#include "Merchant/PW_MerchantComponent.h"
#include "PWInteractableTargetComponent.h"
#include "UI/PWTradePanelWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

APW_MerchantPalCharacter::APW_MerchantPalCharacter()
{
	MerchantComponent = CreateDefaultSubobject<UPW_MerchantComponent>(TEXT("MerchantComponent"));
	InteractableTargetComponent = CreateDefaultSubobject<UPWInteractableTargetComponent>(TEXT("InteractableTargetComponent"));

	if (InteractableTargetComponent)
	{
		InteractableTargetComponent->SetPromptText(NSLOCTEXT("PWMerchant", "DefaultMerchantPrompt", "Talk"));
		InteractableTargetComponent->SetInteractionRadius(250.0f);
		InteractableTargetComponent->SetPriority(20);
	}
}

bool APW_MerchantPalCharacter::CanInteract_Implementation(AActor* Interactor) const
{
	return IsValid(Interactor)
		&& !IsActorBeingDestroyed()
		&& InteractableTargetComponent != nullptr
		&& InteractableTargetComponent->IsInteractionEnabled();
}

bool APW_MerchantPalCharacter::Interact_Implementation(AActor* Interactor)
{
	return CanInteract_Implementation(Interactor);
}

FText APW_MerchantPalCharacter::GetInteractionPrompt_Implementation() const
{
	return InteractableTargetComponent ? InteractableTargetComponent->GetPromptText() : FText::GetEmpty();
}

int32 APW_MerchantPalCharacter::GetInteractionPriority_Implementation() const
{
	return InteractableTargetComponent ? InteractableTargetComponent->GetPriority() : 0;
}

bool APW_MerchantPalCharacter::CanLocalInteract_Implementation(AActor* Interactor) const
{
	return CanInteract_Implementation(Interactor);
}

bool APW_MerchantPalCharacter::LocalInteract_Implementation(AActor* Interactor)
{
	if (!CanLocalInteract_Implementation(Interactor))
	{
		return false;
	}

	if (UPWPlayerTradeComponent* TradeComponent = Interactor->FindComponentByClass<UPWPlayerTradeComponent>())
	{
		if (TradeComponent->OpenTrade(this))
		{
			OpenTradePanel(Interactor, TradeComponent);
			BP_OnMerchantLocalInteracted(Interactor);
			return true;
		}
	}

	return false;
}

bool APW_MerchantPalCharacter::OpenTradePanel(AActor* Interactor, UPWPlayerTradeComponent* TradeComponent)
{
	if (TradePanelWidgetClass == nullptr || Interactor == nullptr || TradeComponent == nullptr)
	{
		return false;
	}

	const APawn* InteractorPawn = Cast<APawn>(Interactor);
	APlayerController* PlayerController = InteractorPawn ? Cast<APlayerController>(InteractorPawn->GetController()) : nullptr;
	if (PlayerController == nullptr || !PlayerController->IsLocalController())
	{
		return false;
	}

	if (TradePanelWidget == nullptr || TradePanelWidget->GetClass() != TradePanelWidgetClass)
	{
		if (TradePanelWidget)
		{
			TradePanelWidget->RemoveFromParent();
		}

		TradePanelWidget = CreateWidget<UPWTradePanelWidget>(PlayerController, TradePanelWidgetClass);
	}

	if (TradePanelWidget == nullptr)
	{
		return false;
	}

	TradePanelWidget->InitializeWithTradeComponent(TradeComponent);
	if (!TradePanelWidget->IsInViewport())
	{
		TradePanelWidget->AddToViewport();
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(TradePanelWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
	TradePanelWidget->SetKeyboardFocus();

	return true;
}
