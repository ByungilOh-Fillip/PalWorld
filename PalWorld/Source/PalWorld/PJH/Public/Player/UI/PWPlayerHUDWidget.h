// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PWPlayerHUDWidget.generated.h"

class APWPlayerCharacter;
class UPWStaminaGaugeWidget;

UCLASS(Blueprintable)
class PALWORLD_API UPWPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|UI")
	void InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Stamina")
	TObjectPtr<UPWStaminaGaugeWidget> StaminaGauge;

private:
	UPROPERTY(Transient)
	TObjectPtr<APWPlayerCharacter> BoundPlayerCharacter;
};
