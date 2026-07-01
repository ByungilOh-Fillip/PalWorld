// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWInventorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Player/Data/PWItemDataAsset.h"
#include "Player/Components/PWPlayerEquipmentComponent.h"
#include "Player/UI/PWInventoryDragDropOperation.h"

void UPWInventorySlotWidget::InitializeInventorySlot(UPWPlayerInventoryLinkComponent* InInventoryComponent, UPWPlayerEquipmentComponent* InEquipmentComponent, const FPWInventorySlotView& InSlotView)
{
	InventoryComponent = InInventoryComponent;
	EquipmentComponent = InEquipmentComponent;
	SlotIndex = InSlotView.SlotIndex;
	bOccupied = InSlotView.bOccupied;
	Count = InSlotView.Count;
	ItemId = InSlotView.ItemId;
	ItemData = InSlotView.ItemData;

	RefreshBoundWidgets();
	BP_OnSlotUpdated();
}

void UPWInventorySlotWidget::RefreshFromInventory()
{
	if (!InventoryComponent || SlotIndex == INDEX_NONE)
	{
		return;
	}

	const FPWInventorySlotView SlotView = InventoryComponent->GetSlotView(SlotIndex);
	InitializeInventorySlot(InventoryComponent, EquipmentComponent, SlotView);
}

FReply UPWInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (bOccupied && EquipmentComponent)
		{
			EquipmentComponent->EquipFromInventorySlotToFirstAvailable(SlotIndex);
			return FReply::Handled();
		}
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bOccupied)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UPWInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!bOccupied || !ItemData)
	{
		return;
	}

	UPWInventoryDragDropOperation* DragOperation = Cast<UPWInventoryDragDropOperation>(
		UWidgetBlueprintLibrary::CreateDragDropOperation(UPWInventoryDragDropOperation::StaticClass()));
	if (!DragOperation)
	{
		return;
	}

	DragOperation->InitializeInventoryPayload(SlotIndex, ItemData, Count);
	DragOperation->DefaultDragVisual = BP_CreateDragVisual();
	DragOperation->Pivot = EDragPivot::MouseDown;
	OutOperation = DragOperation;
}

bool UPWInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UPWInventoryDragDropOperation* DragOperation = Cast<UPWInventoryDragDropOperation>(InOperation);
	if (!DragOperation)
	{
		return false;
	}

	if (DragOperation->IsFromInventory())
	{
		return InventoryComponent && InventoryComponent->MoveItemSlot(DragOperation->SourceSlotIndex, SlotIndex);
	}

	if (DragOperation->IsFromEquipment())
	{
		return EquipmentComponent && EquipmentComponent->UnequipToInventory(DragOperation->SourceSlotIndex);
	}

	return false;
}

void UPWInventorySlotWidget::RefreshBoundWidgets()
{
	const bool bHasItem = bOccupied && ItemData;

	if (Panel_ItemRoot)
	{
		Panel_ItemRoot->SetVisibility(bHasItem ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Image_Icon)
	{
		if (bHasItem && ItemData->GetIcon())
		{
			Image_Icon->SetBrushFromTexture(ItemData->GetIcon(), true);
			Image_Icon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Image_Icon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (Text_Count)
	{
		const bool bShowCount = bHasItem && Count > 1;
		Text_Count->SetText(FText::AsNumber(Count));
		Text_Count->SetVisibility(bShowCount ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Text_Name)
	{
		Text_Name->SetText(bHasItem ? ItemData->GetDisplayName() : FText::GetEmpty());
		Text_Name->SetVisibility(bHasItem ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
