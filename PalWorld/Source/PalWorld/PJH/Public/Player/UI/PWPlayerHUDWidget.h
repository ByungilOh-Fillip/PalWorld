// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PWPlayerHUDWidget.generated.h"

class APWPlayerCharacter;
class UWidget;
class UPWStaminaGaugeWidget;

UCLASS(Blueprintable)
class PALWORLD_API UPWPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|UI")
	void InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter);

	UFUNCTION(BlueprintCallable, Category = "Player|UI|Crosshair")
	void SetCrosshairVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category = "Player|UI|Crosshair")
	bool IsCrosshairVisible() const { return bIsCrosshairVisible; }

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Crosshair", meta = (DisplayName = "On Crosshair Visibility Changed"))
	void BP_OnCrosshairVisibilityChanged(bool bVisible);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Stamina")
	TObjectPtr<UPWStaminaGaugeWidget> StaminaGauge;

	// WBP_PlayerHUD 안에서 크로스헤어 이미지/패널 이름을 CrosshairRoot로 맞추면 C++이 직접 표시 상태를 제어한다.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Crosshair")
	TObjectPtr<UWidget> CrosshairRoot;

private:
	void BroadcastCrosshairVisibility();

	UPROPERTY(Transient)
	TObjectPtr<APWPlayerCharacter> BoundPlayerCharacter;

	UPROPERTY(Transient)
	bool bIsCrosshairVisible = false;
};
