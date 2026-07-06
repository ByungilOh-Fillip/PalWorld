#include "UI/PWTradePanelWidget.h"

#include "Merchant/PWPlayerTradeComponent.h"
#include "Merchant/PW_MerchantPalCharacter.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"

void UPWTradePanelWidget::InitializeWithTradeComponent(UPWPlayerTradeComponent* InTradeComponent)
{
	if (BoundTradeComponent == InTradeComponent)
	{
		HandleTradeStateChanged();
		return;
	}

	UnbindTradeComponent();
	BoundTradeComponent = InTradeComponent;

	if (BoundTradeComponent)
	{
		BoundTradeComponent->OnTradeStateChanged.AddUniqueDynamic(this, &UPWTradePanelWidget::HandleTradeStateChanged);
	}

	HandleTradeStateChanged();
}

APW_MerchantPalCharacter* UPWTradePanelWidget::GetCurrentMerchant() const
{
	return BoundTradeComponent ? BoundTradeComponent->GetCurrentMerchant() : nullptr;
}

TArray<FPW_MerchantTradeEntry> UPWTradePanelWidget::GetTradeEntries() const
{
	const APW_MerchantPalCharacter* Merchant = GetCurrentMerchant();
	const UPW_MerchantComponent* MerchantComponent = Merchant ? Merchant->GetMerchantComponent() : nullptr;
	return MerchantComponent ? MerchantComponent->GetTradeEntries() : TArray<FPW_MerchantTradeEntry>();
}

bool UPWTradePanelWidget::RequestBuyItem(FName ItemId, int32 Count)
{
	return BoundTradeComponent && BoundTradeComponent->RequestBuyItem(ItemId, Count);
}

bool UPWTradePanelWidget::RequestSellItem(FName ItemId, int32 Count)
{
	return BoundTradeComponent && BoundTradeComponent->RequestSellItem(ItemId, Count);
}

void UPWTradePanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Close)
	{
		Button_Close->OnClicked.AddUniqueDynamic(this, &UPWTradePanelWidget::HandleCloseButtonClicked);
	}

	HandleTradeStateChanged();
}

void UPWTradePanelWidget::NativeDestruct()
{
	if (Button_Close)
	{
		Button_Close->OnClicked.RemoveDynamic(this, &UPWTradePanelWidget::HandleCloseButtonClicked);
	}

	RestoreGameInputMode();
	UnbindTradeComponent();
	Super::NativeDestruct();
}

void UPWTradePanelWidget::HandleTradeStateChanged()
{
	RefreshVisibility();
	BP_OnTradeStateChanged();
}

void UPWTradePanelWidget::HandleCloseButtonClicked()
{
	if (BoundTradeComponent)
	{
		BoundTradeComponent->CloseTrade();
	}
}

void UPWTradePanelWidget::UnbindTradeComponent()
{
	if (BoundTradeComponent)
	{
		BoundTradeComponent->OnTradeStateChanged.RemoveDynamic(this, &UPWTradePanelWidget::HandleTradeStateChanged);
		BoundTradeComponent = nullptr;
	}
}

void UPWTradePanelWidget::RefreshVisibility()
{
	const bool bHasMerchant = GetCurrentMerchant() != nullptr;
	SetVisibility(bHasMerchant ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (!bHasMerchant)
	{
		RestoreGameInputMode();
	}
}

void UPWTradePanelWidget::RestoreGameInputMode()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (PlayerController == nullptr || !PlayerController->IsLocalController())
	{
		return;
	}

	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = false;
}
