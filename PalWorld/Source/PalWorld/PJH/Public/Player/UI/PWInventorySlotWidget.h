// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Components/PWPlayerInventoryLinkComponent.h"
#include "PWInventorySlotWidget.generated.h"

class UPWItemDataAsset;
class UPWPlayerEquipmentComponent;
class UPWPlayerInventoryLinkComponent;
class UImage;
class UTextBlock;
class UWidget;

UCLASS(Blueprintable)
class PALWORLD_API UPWInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|UI|Inventory")
	void InitializeInventorySlot(UPWPlayerInventoryLinkComponent* InInventoryComponent, UPWPlayerEquipmentComponent* InEquipmentComponent, const FPWInventorySlotView& InSlotView);

	UFUNCTION(BlueprintCallable, Category = "Player|UI|Inventory")
	void RefreshFromInventory();

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	bool IsOccupied() const { return bOccupied; }

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	int32 GetSlotIndex() const { return SlotIndex; }

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	int32 GetCount() const { return Count; }

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	UPWItemDataAsset* GetItemData() const { return ItemData; }

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Inventory", meta = (DisplayName = "On Slot Updated"))
	void BP_OnSlotUpdated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Inventory", meta = (DisplayName = "Create Drag Visual"))
	UWidget* BP_CreateDragVisual();

	UPROPERTY(BlueprintReadOnly, Category = "Player|UI|Inventory")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Player|UI|Inventory")
	bool bOccupied = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|UI|Inventory")
	int32 Count = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Player|UI|Inventory")
	TObjectPtr<UPWItemDataAsset> ItemData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Player|UI|Inventory")
	FName ItemId = NAME_None;

	// WBP_InventorySlot에서 이름을 맞춰두면 C++이 기본 아이콘/수량 표시를 자동으로 처리한다.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UImage> Image_Icon = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UTextBlock> Text_Count = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UTextBlock> Text_Name = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UWidget> Panel_ItemRoot = nullptr;

private:
	void RefreshBoundWidgets();

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerInventoryLinkComponent> InventoryComponent;

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerEquipmentComponent> EquipmentComponent;
};
