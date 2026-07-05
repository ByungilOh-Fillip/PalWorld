#include "Merchant/PW_MerchantPalCharacter.h"

#include "Merchant/PWPlayerTradeComponent.h"
#include "Merchant/PW_MerchantComponent.h"
#include "PWInteractableTargetComponent.h"

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
			BP_OnMerchantLocalInteracted(Interactor);
			return true;
		}
	}

	return false;
}
