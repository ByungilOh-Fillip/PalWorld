// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PWPlayerHUDWidget.generated.h"

class APWPlayerCharacter;
class UProgressBar;
class UTextBlock;
class UWidget;
class UPWInventoryPanelWidget;
class UPWStaminaGaugeWidget;
class UPWPlayerStatComponent;

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

	UFUNCTION(BlueprintCallable, Category = "Player|UI|Inventory")
	void SetInventoryVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	bool IsInventoryVisible() const { return bIsInventoryVisible; }

	UFUNCTION(BlueprintPure, Category = "Player|UI|Capture")
	bool IsCaptureAimVisible() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Capture")
	bool HasCaptureAimTarget() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Capture")
	float GetCaptureAimChance() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Capture")
	int32 GetCaptureAimChancePercent() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Capture")
	FText GetCaptureAimTargetNameText() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Capture")
	int32 GetCaptureSphereCount() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Crosshair", meta = (DisplayName = "On Crosshair Visibility Changed"))
	void BP_OnCrosshairVisibilityChanged(bool bVisible);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Inventory", meta = (DisplayName = "On Inventory Visibility Changed"))
	void BP_OnInventoryVisibilityChanged(bool bVisible);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Stats", meta = (DisplayName = "On Survival Stats Changed"))
	void BP_OnSurvivalStatsChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Capture", meta = (DisplayName = "On Capture Aim Changed"))
	void BP_OnCaptureAimChanged();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Stamina")
	TObjectPtr<UPWStaminaGaugeWidget> StaminaGauge;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Stats")
	TObjectPtr<UProgressBar> Progress_Health;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Stats")
	TObjectPtr<UProgressBar> Progress_Shield;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Stats")
	TObjectPtr<UWidget> ShieldRoot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Stats")
	TObjectPtr<UProgressBar> Progress_Hunger;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Stats")
	TObjectPtr<UTextBlock> Text_Health;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Stats")
	TObjectPtr<UTextBlock> Text_Shield;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Stats")
	TObjectPtr<UTextBlock> Text_Hunger;

	// WBP_PlayerHUD 안에서 인벤토리 패널 이름을 InventoryPanel로 맞추면 C++이 초기화와 표시 상태를 제어한다.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory")
	TObjectPtr<UPWInventoryPanelWidget> InventoryPanel;

	// HUD 위젯 트리에 패널을 직접 넣지 않아도, 여기 클래스를 지정하면 런타임에 별도 위젯으로 생성한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|UI|Inventory")
	TSubclassOf<UPWInventoryPanelWidget> InventoryPanelWidgetClass;

	// WBP_PlayerHUD 안에서 크로스헤어 이미지/패널 이름을 CrosshairRoot로 맞추면 C++이 직접 표시 상태를 제어한다.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Crosshair")
	TObjectPtr<UWidget> CrosshairRoot;

private:
	UFUNCTION()
	void HandleSurvivalStatsChanged();

	void BroadcastCrosshairVisibility();
	void BroadcastInventoryVisibility();
	void BindStatComponent(UPWPlayerStatComponent* InStatComponent);
	void UnbindStatComponent();
	void BindCaptureComponent();
	void UnbindCaptureComponent();
	void RefreshSurvivalStats();
	void BroadcastCaptureAimChanged();
	UPWInventoryPanelWidget* GetOrCreateInventoryPanel();

	UFUNCTION()
	void HandleCaptureAimInfoChanged();

	UPROPERTY(Transient)
	TObjectPtr<APWPlayerCharacter> BoundPlayerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerStatComponent> BoundStatComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UPWPlayerCaptureComponent> BoundCaptureComponent;

	UPROPERTY(Transient)
	bool bIsCrosshairVisible = false;

	UPROPERTY(Transient)
	bool bIsInventoryVisible = false;

	UPROPERTY(Transient)
	TObjectPtr<UPWInventoryPanelWidget> CreatedInventoryPanel;
};
