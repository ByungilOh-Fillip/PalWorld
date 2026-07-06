// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWEquipmentSlotWidget.h"

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
#include "Player/Components/PWPlayerEquipmentComponent.h"
#include "Player/Components/PWPlayerInventoryLinkComponent.h"
#include "Player/Data/PWItemDataAsset.h"
#include "Player/UI/PWInventoryDragDropOperation.h"

void UPWEquipmentSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	RefreshBoundWidgets();
}

FReply UPWEquipmentSlotWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FReply SlotReply = HandleSlotMouseButtonDown(InGeometry, InMouseEvent);
	return SlotReply.IsEventHandled()
		? SlotReply
		: Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

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
	Count = 0;

	if (EquipmentComponent && EquipmentSlotIndex != INDEX_NONE)
	{
		const FPWEquipmentSlotData& SlotData = EquipmentComponent->GetSlotData(EquipmentSlotIndex);
		ItemData = SlotData.ItemData;
		Count = SlotData.Count;
	}

	BP_OnSlotUpdated();
	RefreshBoundWidgets();
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
	const FReply SlotReply = HandleSlotMouseButtonDown(InGeometry, InMouseEvent);
	return SlotReply.IsEventHandled()
		? SlotReply
		: Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UPWEquipmentSlotWidget::HandleSlotMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
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

	return FReply::Unhandled();
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
	if (!DragOperation->DefaultDragVisual)
	{
		DragOperation->DefaultDragVisual = CreateDefaultDragVisual();
	}
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

UWidget* UPWEquipmentSlotWidget::CreateDefaultDragVisual() const
{
	if (!ItemData)
	{
		return nullptr;
	}

	USizeBox* DragRoot = NewObject<USizeBox>(const_cast<UPWEquipmentSlotWidget*>(this));
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
			Image_Icon->SetColorAndOpacity(FLinearColor::White);
			Image_Icon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else if (bHasItem)
		{
			// 아이콘이 아직 없는 테스트 장비도 장착 여부를 확인할 수 있게 기본 표시를 남긴다.
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

	if (Text_Name)
	{
		Text_Name->SetText(bHasItem ? ItemData->GetDisplayName() : FText::GetEmpty());
		Text_Name->SetVisibility(bHasItem ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Text_Count)
	{
		Text_Count->SetText(FText::AsNumber(Count));
		Text_Count->SetVisibility(bHasItem && Count > 1 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Panel_Selected)
	{
		Panel_Selected->SetVisibility(IsSelected() ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
