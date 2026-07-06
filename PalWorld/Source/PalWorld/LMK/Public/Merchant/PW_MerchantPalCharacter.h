#pragma once

#include "CoreMinimal.h"
#include "PWInteractable.h"
#include "PWLocalInteractable.h"
#include "PWPalCharacter.h"
#include "PW_MerchantPalCharacter.generated.h"

class UPWInteractableTargetComponent;
class UPW_MerchantComponent;
class UPWPlayerTradeComponent;
class UPWTradePanelWidget;

UCLASS(Blueprintable)
class PALWORLD_API APW_MerchantPalCharacter : public APWPalCharacter, public IPWInteractable, public IPWLocalInteractable
{
	GENERATED_BODY()

public:
	APW_MerchantPalCharacter();

	UFUNCTION(BlueprintPure, Category = "PW|Merchant")
	UPW_MerchantComponent* GetMerchantComponent() const { return MerchantComponent; }

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	UPWInteractableTargetComponent* GetInteractableTargetComponent() const { return InteractableTargetComponent; }

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual bool Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual int32 GetInteractionPriority_Implementation() const override;
	virtual bool CanLocalInteract_Implementation(AActor* Interactor) const override;
	virtual bool LocalInteract_Implementation(AActor* Interactor) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Merchant", meta = (DisplayName = "On Merchant Local Interacted"))
	void BP_OnMerchantLocalInteracted(AActor* Interactor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Merchant|UI")
	TSubclassOf<UPWTradePanelWidget> TradePanelWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Merchant")
	TObjectPtr<UPW_MerchantComponent> MerchantComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Interaction")
	TObjectPtr<UPWInteractableTargetComponent> InteractableTargetComponent;

private:
	bool OpenTradePanel(AActor* Interactor, UPWPlayerTradeComponent* TradeComponent);

	UPROPERTY(Transient)
	TObjectPtr<UPWTradePanelWidget> TradePanelWidget;
};
