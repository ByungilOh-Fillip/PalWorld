// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Types/PWPlayerGameplayTypes.h"
#include "PWEquipmentSlotWidget.generated.h"

class UPWItemDataAsset;
class UPWPlayerEquipmentComponent;
class UPWPlayerInventoryLinkComponent;
class UImage;
class UTextBlock;
class UWidget;

UCLASS(Blueprintable)
class PALWORLD_API UPWEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|UI|Equipment")
	void InitializeEquipmentSlot(UPWPlayerEquipmentComponent* InEquipmentComponent, UPWPlayerInventoryLinkComponent* InInventoryComponent, int32 InSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Player|UI|Equipment")
	void RefreshFromEquipment();

	UFUNCTION(BlueprintPure, Category = "Player|UI|Equipment")
	int32 GetEquipmentSlotIndex() const { return EquipmentSlotIndex; }

	UFUNCTION(BlueprintPure, Category = "Player|UI|Equipment")
	bool IsOccupied() const { return ItemData != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Player|UI|Equipment")
	bool IsSelected() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Equipment")
	EPWEquipmentSlotType GetSlotType() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Equipment")
	UPWItemDataAsset* GetItemData() const { return ItemData; }

	UFUNCTION(BlueprintPure, Category = "Player|UI|Equipment")
	int32 GetItemCount() const { return Count; }

protected:
	virtual void NativePreConstruct() override;
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Equipment", meta = (DisplayName = "On Slot Updated"))
	void BP_OnSlotUpdated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Equipment", meta = (DisplayName = "Create Drag Visual"))
	UWidget* BP_CreateDragVisual();

	UPROPERTY(BlueprintReadOnly, Category = "Player|UI|Equipment")
	int32 EquipmentSlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Player|UI|Equipment")
	TObjectPtr<UPWItemDataAsset> ItemData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Player|UI|Equipment")
	int32 Count = 0;

	// WBP_EquipmentSlot에서 이름을 맞춰두면 C++이 기본 아이콘/선택 표시를 자동으로 처리한다.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Equipment|Bind")
	TObjectPtr<UImage> Image_Icon = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Equipment|Bind")
	TObjectPtr<UTextBlock> Text_Name = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Equipment|Bind")
	TObjectPtr<UTextBlock> Text_Count = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Equipment|Bind")
	TObjectPtr<UWidget> Panel_ItemRoot = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Equipment|Bind")
	TObjectPtr<UWidget> Panel_Selected = nullptr;

private:
	FReply HandleSlotMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	UWidget* CreateDefaultDragVisual() const;
	void RefreshBoundWidgets();

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerEquipmentComponent> EquipmentComponent;

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerInventoryLinkComponent> InventoryComponent;
};
