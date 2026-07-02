// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWInventorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Player/Data/PWItemDataAsset.h"
#include "Player/Components/PWPlayerEquipmentComponent.h"
#include "Player/UI/PWInventoryDragDropOperation.h"

void UPWInventorySlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	RefreshBoundWidgets();
}

FReply UPWInventorySlotWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FReply SlotReply = HandleSlotMouseButtonDown(InGeometry, InMouseEvent);
	return SlotReply.IsEventHandled()
		? SlotReply
		: Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

void UPWInventorySlotWidget::InitializeInventorySlot(UPWPlayerInventoryLinkComponent* InInventoryComponent, UPWPlayerEquipmentComponent* InEquipmentComponent, const FPWInventorySlotView& InSlotView)
{
	InventoryComponent = InInventoryComponent;
	EquipmentComponent = InEquipmentComponent;
	SlotIndex = InSlotView.SlotIndex;
	bOccupied = InSlotView.bOccupied;
	Count = InSlotView.Count;
	ItemId = InSlotView.ItemId;
	ItemData = InSlotView.ItemData;

	BP_OnSlotUpdated();
	RefreshBoundWidgets();
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
	const FReply SlotReply = HandleSlotMouseButtonDown(InGeometry, InMouseEvent);
	return SlotReply.IsEventHandled()
		? SlotReply
		: Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UPWInventorySlotWidget::HandleSlotMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
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

	return FReply::Unhandled();
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
	if (!DragOperation->DefaultDragVisual)
	{
		DragOperation->DefaultDragVisual = CreateDefaultDragVisual();
	}
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
		return EquipmentComponent && EquipmentComponent->UnequipToInventorySlot(DragOperation->SourceSlotIndex, SlotIndex);
	}

	return false;
}

UWidget* UPWInventorySlotWidget::CreateDefaultDragVisual() const
{
	if (!ItemData)
	{
		return nullptr;
	}

	USizeBox* DragRoot = NewObject<USizeBox>(const_cast<UPWInventorySlotWidget*>(this));
	UOverlay* DragOverlay = NewObject<UOverlay>(DragRoot);
	UImage* DragIcon = NewObject<UImage>(DragOverlay);
	UTextBlock* DragCountText = NewObject<UTextBlock>(DragOverlay);
	UTextBlock* DragNameText = NewObject<UTextBlock>(DragOverlay);
	if (!DragRoot || !DragOverlay || !DragIcon || !DragCountText || !DragNameText)
	{
		return nullptr;
	}

	DragRoot->SetWidthOverride(72.f);
	DragRoot->SetHeightOverride(72.f);
	DragRoot->SetContent(DragOverlay);

	if (ItemData->GetIcon())
	{
		DragIcon->SetBrushFromTexture(ItemData->GetIcon(), true);
		DragIcon->SetColorAndOpacity(FLinearColor::White);
	}
	else
	{
		DragIcon->SetColorAndOpacity(FLinearColor(0.12f, 0.65f, 0.85f, 0.9f));
	}

	if (UOverlaySlot* IconSlot = DragOverlay->AddChildToOverlay(DragIcon))
	{
		IconSlot->SetHorizontalAlignment(HAlign_Fill);
		IconSlot->SetVerticalAlignment(VAlign_Fill);
	}

	DragCountText->SetText(FText::AsNumber(Count));
	DragCountText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	DragCountText->SetShadowOffset(FVector2D(1.f, 1.f));
	DragCountText->SetVisibility(Count > 1 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (UOverlaySlot* CountSlot = DragOverlay->AddChildToOverlay(DragCountText))
	{
		CountSlot->SetHorizontalAlignment(HAlign_Right);
		CountSlot->SetVerticalAlignment(VAlign_Bottom);
		CountSlot->SetPadding(FMargin(0.f, 0.f, 4.f, 2.f));
	}

	DragNameText->SetText(ItemData->GetDisplayName());
	DragNameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	DragNameText->SetShadowOffset(FVector2D(1.f, 1.f));
	DragNameText->SetVisibility(ItemData->GetIcon() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	if (UOverlaySlot* NameSlot = DragOverlay->AddChildToOverlay(DragNameText))
	{
		NameSlot->SetHorizontalAlignment(HAlign_Center);
		NameSlot->SetVerticalAlignment(VAlign_Center);
	}

	return DragRoot;
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
			Image_Icon->SetColorAndOpacity(FLinearColor::White);
			Image_Icon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else if (bHasItem)
		{
			// 아이콘이 아직 없는 테스트 아이템도 슬롯 안에서 위치를 확인할 수 있게 기본 표시를 남긴다.
			Image_Icon->SetColorAndOpacity(FLinearColor(0.12f, 0.65f, 0.85f, 0.85f));
			Image_Icon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Image_Icon->SetBrush(FSlateBrush());
			Image_Icon->SetColorAndOpacity(FLinearColor::Transparent);
			Image_Icon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (Text_Count)
	{
		const bool bShowCount = bHasItem && (Count > 1 || !ItemData->GetIcon());
		Text_Count->SetText(FText::AsNumber(Count));
		Text_Count->SetVisibility(bShowCount ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Text_Name)
	{
		Text_Name->SetText(bHasItem ? ItemData->GetDisplayName() : FText::GetEmpty());
		Text_Name->SetVisibility(bHasItem ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
