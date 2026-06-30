// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Components/PWPlayerInventoryLinkComponent.h"
#include "PWInventoryPanelWidget.generated.h"

class UPWPlayerInventoryLinkComponent;
class UTextBlock;
class UUniformGridPanel;
class UWidget;

UCLASS(Blueprintable)
class PALWORLD_API UPWInventoryPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|UI|Inventory")
	void InitializeWithInventoryComponent(UPWPlayerInventoryLinkComponent* InInventoryComponent);

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	UPWPlayerInventoryLinkComponent* GetInventoryComponent() const { return BoundInventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	TArray<FPWInventorySlotView> GetSlotViews() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	int32 GetInventorySlotCount() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	float GetMaxCarryWeight() const;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditDefaultsOnly, Category = "Player|UI|Inventory")
	bool bUseDefaultRuntimeLayout = true;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Inventory", meta = (DisplayName = "On Inventory Changed"))
	void BP_OnInventoryChanged();

private:
	UFUNCTION()
	void HandleInventoryChanged();

	void BuildDefaultLayout();
	void RefreshDefaultLayout();
	UWidget* CreateDefaultSlotWidget(const FPWInventorySlotView& SlotView);
	void UnbindInventoryComponent();

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerInventoryLinkComponent> BoundInventoryComponent;

	UPROPERTY(Transient)
	TObjectPtr<UUniformGridPanel> DefaultInventoryGrid;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DefaultWeightText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DefaultTitleText;

	UPROPERTY(Transient)
	bool bUsingDefaultLayout = false;
};
