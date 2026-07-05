#include "Merchant/PWPlayerTradeComponent.h"

#include "Merchant/PW_MerchantComponent.h"
#include "Merchant/PW_MerchantPalCharacter.h"
#include "GameFramework/Actor.h"

UPWPlayerTradeComponent::UPWPlayerTradeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UPWPlayerTradeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bTradeOpen && !CanTradeWithMerchant(CurrentMerchant.Get()))
	{
		CloseTrade();
	}
}

bool UPWPlayerTradeComponent::OpenTrade(APW_MerchantPalCharacter* Merchant)
{
	if (!CanTradeWithMerchant(Merchant))
	{
		return false;
	}

	CurrentMerchant = Merchant;
	bTradeOpen = true;
	BroadcastTradeStateChanged();
	return true;
}

void UPWPlayerTradeComponent::CloseTrade()
{
	if (!bTradeOpen && !CurrentMerchant.IsValid())
	{
		return;
	}

	CurrentMerchant.Reset();
	bTradeOpen = false;
	BroadcastTradeStateChanged();
}

bool UPWPlayerTradeComponent::CanTradeWithMerchant(APW_MerchantPalCharacter* Merchant) const
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !IsValid(Merchant) || Merchant->GetMerchantComponent() == nullptr)
	{
		return false;
	}

	return FVector::DistSquared(Owner->GetActorLocation(), Merchant->GetActorLocation()) <= FMath::Square(MaxTradeDistance);
}

bool UPWPlayerTradeComponent::RequestBuyItem(FName ItemId, int32 Count)
{
	APW_MerchantPalCharacter* Merchant = CurrentMerchant.Get();
	if (!IsValidTradeRequest(Merchant, ItemId, Count))
	{
		return false;
	}

	if (GetOwner() != nullptr && !GetOwner()->HasAuthority())
	{
		ServerBuyItem(Merchant, ItemId, Count);
		return true;
	}

	HandleBuyItemAuthority(Merchant, ItemId, Count);
	return true;
}

bool UPWPlayerTradeComponent::RequestSellItem(FName ItemId, int32 Count)
{
	APW_MerchantPalCharacter* Merchant = CurrentMerchant.Get();
	if (!IsValidTradeRequest(Merchant, ItemId, Count))
	{
		return false;
	}

	if (GetOwner() != nullptr && !GetOwner()->HasAuthority())
	{
		ServerSellItem(Merchant, ItemId, Count);
		return true;
	}

	HandleSellItemAuthority(Merchant, ItemId, Count);
	return true;
}

void UPWPlayerTradeComponent::ServerBuyItem_Implementation(APW_MerchantPalCharacter* Merchant, FName ItemId, int32 Count)
{
	HandleBuyItemAuthority(Merchant, ItemId, Count);
}

void UPWPlayerTradeComponent::ServerSellItem_Implementation(APW_MerchantPalCharacter* Merchant, FName ItemId, int32 Count)
{
	HandleSellItemAuthority(Merchant, ItemId, Count);
}

bool UPWPlayerTradeComponent::IsValidTradeRequest(APW_MerchantPalCharacter* Merchant, FName ItemId, int32 Count) const
{
	if (!CanTradeWithMerchant(Merchant) || ItemId.IsNone() || Count <= 0)
	{
		return false;
	}

	const UPW_MerchantComponent* MerchantComponent = Merchant->GetMerchantComponent();
	FPW_MerchantTradeEntry TradeEntry;
	return MerchantComponent != nullptr && MerchantComponent->FindTradeEntry(ItemId, TradeEntry);
}

void UPWPlayerTradeComponent::HandleBuyItemAuthority(APW_MerchantPalCharacter* Merchant, FName ItemId, int32 Count)
{
	if (!IsValidTradeRequest(Merchant, ItemId, Count))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[PWTrade] Buy request accepted for future implementation. Merchant=%s ItemId=%s Count=%d"),
		*GetNameSafe(Merchant),
		*ItemId.ToString(),
		Count);
}

void UPWPlayerTradeComponent::HandleSellItemAuthority(APW_MerchantPalCharacter* Merchant, FName ItemId, int32 Count)
{
	if (!IsValidTradeRequest(Merchant, ItemId, Count))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[PWTrade] Sell request accepted for future implementation. Merchant=%s ItemId=%s Count=%d"),
		*GetNameSafe(Merchant),
		*ItemId.ToString(),
		Count);
}

void UPWPlayerTradeComponent::BroadcastTradeStateChanged()
{
	OnTradeStateChanged.Broadcast();
}
