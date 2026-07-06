#include "Merchant/PW_MerchantComponent.h"

#include "Player/Data/PWItemDataAsset.h"

bool FPW_MerchantTradeEntry::MatchesItem(FName InItemId) const
{
	if (InItemId.IsNone())
	{
		return false;
	}

	if (ItemId == InItemId)
	{
		return true;
	}

	return ItemData != nullptr && ItemData->GetItemId() == InItemId;
}

UPW_MerchantComponent::UPW_MerchantComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPW_MerchantComponent::FindTradeEntry(FName ItemId, FPW_MerchantTradeEntry& OutTradeEntry) const
{
	for (const FPW_MerchantTradeEntry& TradeEntry : TradeEntries)
	{
		if (TradeEntry.MatchesItem(ItemId))
		{
			OutTradeEntry = TradeEntry;
			return true;
		}
	}

	return false;
}
