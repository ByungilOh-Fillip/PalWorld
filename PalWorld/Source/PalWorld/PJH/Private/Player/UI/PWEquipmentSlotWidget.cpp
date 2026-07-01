// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWEquipmentSlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Player/Components/PWPlayerEquipmentComponent.h"
#include "Player/Data/PWItemDataAsset.h"
#include "Player/UI/PWInventoryDragDropOperation.h"

void UPWEquipmentSlotWidget::InitializeEquipmentSlot(UPWPlayerEquipmentComponent* InEquipmentComponent, UPWPlayerInventoryLinkComponent* InInventoryComponent, int32 InSlotIndex)
{
	EquipmentComponent = InEquipmentComponent;
	InventoryComponent = InInventoryComponent;
	EquipmentSlotIndex = InSlotIndex;
	RefreshFromEquipment();
}

void UPWEquipmentSlotWidget::RefreshFromEquipment()
{
	ItemData = nullptr;

	if (EquipmentComponent && EquipmentSlotIndex != INDEX_NONE)
	{
		ItemData = EquipmentComponent->GetSlotData(EquipmentSlotIndex).ItemData;
	}

	RefreshBoundWidgets();
	BP_OnSlotUpdated();
}

bool UPWEquipmentSlotWidget::IsSelected() const
{
	return EquipmentComponent && EquipmentComponent->GetSelectedSlotIndex() == EquipmentSlotIndex;
}

EPWEquipmentSlotType UPWEquipmentSlotWidget::GetSlotType() const
{
	return EquipmentComponent
		? EquipmentComponent->GetSlotType(EquipmentSlotIndex)
		: EPWEquipmentSlotType::None;
}

FReply UPWEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (ItemData && EquipmentComponent)
		{
			EquipmentComponent->UnequipToInventory(EquipmentSlotIndex);
			return FReply::Handled();
		}
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && ItemData)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UPWEquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!ItemData)
	{
		return;
	}

	UPWInventoryDragDropOperation* DragOperation = Cast<UPWInventoryDragDropOperation>(
		UWidgetBlueprintLibrary::CreateDragDropOperation(UPWInventoryDragDropOperation::StaticClass()));
	if (!DragOperation)
	{
		return;
	}

	DragOperation->InitializeEquipmentPayload(EquipmentSlotIndex, ItemData);
	DragOperation->DefaultDragVisual = BP_CreateDragVisual();
	DragOperation->Pivot = EDragPivot::MouseDown;
	OutOperation = DragOperation;
}

bool UPWEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UPWInventoryDragDropOperation* DragOperation = Cast<UPWInventoryDragDropOperation>(InOperation);
	if (!DragOperation || !EquipmentComponent)
	{
		return false;
	}

	if (DragOperation->IsFromInventory())
	{
		return EquipmentComponent->EquipFromInventorySlot(DragOperation->SourceSlotIndex, EquipmentSlotIndex);
	}

	if (DragOperation->IsFromEquipment())
	{
		return EquipmentComponent->MoveEquipmentSlot(DragOperation->SourceSlotIndex, EquipmentSlotIndex);
	}

	return false;
}

void UPWEquipmentSlotWidget::RefreshBoundWidgets()
{
	const bool bHasItem = ItemData != nullptr;

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

	if (Text_Name)
	{
		Text_Name->SetText(bHasItem ? ItemData->GetDisplayName() : FText::GetEmpty());
		Text_Name->SetVisibility(bHasItem ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Panel_Selected)
	{
		Panel_Selected->SetVisibility(IsSelected() ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
