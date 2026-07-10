#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PW_OptionsWidget.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPW_OnOptionsClosed);

UCLASS(Blueprintable)
class PALWORLD_API UPW_OptionsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintAssignable, Category = "PW|Options")
	FPW_OnOptionsClosed OnOptionsClosed;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_CloseOptions;

	UFUNCTION()
	void HandleCloseClicked();
};
