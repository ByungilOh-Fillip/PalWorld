#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Merchant/PW_MerchantComponent.h"
#include "PWTradePanelWidget.generated.h"

class APW_MerchantPalCharacter;
class UPWPlayerTradeComponent;

UCLASS(Blueprintable)
class PALWORLD_API UPWTradePanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PW|Trade")
	void InitializeWithTradeComponent(UPWPlayerTradeComponent* InTradeComponent);

	UFUNCTION(BlueprintPure, Category = "PW|Trade")
	UPWPlayerTradeComponent* GetTradeComponent() const { return BoundTradeComponent; }

	UFUNCTION(BlueprintPure, Category = "PW|Trade")
	APW_MerchantPalCharacter* GetCurrentMerchant() const;

	UFUNCTION(BlueprintPure, Category = "PW|Trade")
	TArray<FPW_MerchantTradeEntry> GetTradeEntries() const;

	UFUNCTION(BlueprintCallable, Category = "PW|Trade")
	bool RequestBuyItem(FName ItemId, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "PW|Trade")
	bool RequestSellItem(FName ItemId, int32 Count);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Trade", meta = (DisplayName = "On Trade State Changed"))
	void BP_OnTradeStateChanged();

private:
	UFUNCTION()
	void HandleTradeStateChanged();

	void UnbindTradeComponent();
	void RefreshVisibility();

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerTradeComponent> BoundTradeComponent;
};
