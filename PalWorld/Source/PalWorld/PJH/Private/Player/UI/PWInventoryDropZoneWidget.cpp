// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWInventoryDropZoneWidget.h"

#include "Player/Components/PWPlayerEquipmentComponent.h"
#include "Player/Components/PWPlayerInventoryLinkComponent.h"
#include "Player/UI/PWInventoryDragDropOperation.h"

void UPWInventoryDropZoneWidget::InitializeDropZone(UPWPlayerInventoryLinkComponent* InInventoryComponent, UPWPlayerEquipmentComponent* InEquipmentComponent)
{
	InventoryComponent = InInventoryComponent;
	EquipmentComponent = InEquipmentComponent;
}

bool UPWInventoryDropZoneWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UPWInventoryDragDropOperation* DragOperation = Cast<UPWInventoryDragDropOperation>(InOperation);
	if (!DragOperation)
	{
		return false;
	}

	if (DragOperation->IsFromInventory())
	{
		if (!InventoryComponent)
		{
			return false;
		}

		return ZoneType == EPWInventoryDropZoneType::Destroy
			? InventoryComponent->DestroyItemFromSlot(DragOperation->SourceSlotIndex, DragOperation->Count)
			: InventoryComponent->DropItemFromSlot(DragOperation->SourceSlotIndex, DragOperation->Count);
	}

	if (DragOperation->IsFromEquipment())
	{
		if (!EquipmentComponent)
		{
			return false;
		}

		return ZoneType == EPWInventoryDropZoneType::Destroy
			? EquipmentComponent->DestroyEquipmentSlot(DragOperation->SourceSlotIndex)
			: EquipmentComponent->DropEquipmentSlot(DragOperation->SourceSlotIndex);
	}

	return false;
}
