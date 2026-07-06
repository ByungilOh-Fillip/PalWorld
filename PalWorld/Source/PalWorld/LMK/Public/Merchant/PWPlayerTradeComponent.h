#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWPlayerTradeComponent.generated.h"

class APW_MerchantPalCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPW_OnTradeStateChanged);

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerTradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerTradeComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Player|Trade")
	bool OpenTrade(APW_MerchantPalCharacter* Merchant);

	UFUNCTION(BlueprintCallable, Category = "Player|Trade")
	void CloseTrade();

	UFUNCTION(BlueprintPure, Category = "Player|Trade")
	APW_MerchantPalCharacter* GetCurrentMerchant() const { return CurrentMerchant.Get(); }

	UFUNCTION(BlueprintPure, Category = "Player|Trade")
	bool CanTradeWithMerchant(APW_MerchantPalCharacter* Merchant) const;

	UFUNCTION(BlueprintCallable, Category = "Player|Trade")
	bool RequestBuyItem(FName ItemId, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Player|Trade")
	bool RequestSellItem(FName ItemId, int32 Count);

	UPROPERTY(BlueprintAssignable, Category = "Player|Trade")
	FPW_OnTradeStateChanged OnTradeStateChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Trade", meta = (ClampMin = "0.0"))
	float MaxTradeDistance = 350.0f;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<APW_MerchantPalCharacter> CurrentMerchant;

	UPROPERTY(Transient)
	bool bTradeOpen = false;

	UFUNCTION(Server, Reliable)
	void ServerBuyItem(APW_MerchantPalCharacter* Merchant, FName ItemId, int32 Count);

	UFUNCTION(Server, Reliable)
	void ServerSellItem(APW_MerchantPalCharacter* Merchant, FName ItemId, int32 Count);

	bool IsValidTradeRequest(APW_MerchantPalCharacter* Merchant, FName ItemId, int32 Count) const;
	void HandleBuyItemAuthority(APW_MerchantPalCharacter* Merchant, FName ItemId, int32 Count);
	void HandleSellItemAuthority(APW_MerchantPalCharacter* Merchant, FName ItemId, int32 Count);
	void BroadcastTradeStateChanged();
};
