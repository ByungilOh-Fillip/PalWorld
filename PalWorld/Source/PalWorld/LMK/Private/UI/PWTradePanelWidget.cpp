#include "UI/PWTradePanelWidget.h"

#include "Merchant/PWPlayerTradeComponent.h"
#include "Merchant/PW_MerchantPalCharacter.h"

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
	HandleTradeStateChanged();
}

void UPWTradePanelWidget::NativeDestruct()
{
	UnbindTradeComponent();
	Super::NativeDestruct();
}

void UPWTradePanelWidget::HandleTradeStateChanged()
{
	RefreshVisibility();
	BP_OnTradeStateChanged();
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
	SetVisibility(GetCurrentMerchant() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
