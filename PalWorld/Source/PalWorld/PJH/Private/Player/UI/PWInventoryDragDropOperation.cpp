// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWInventoryDragDropOperation.h"

void UPWInventoryDragDropOperation::InitializeInventoryPayload(int32 InSourceSlotIndex, UPWItemDataAsset* InItemData, int32 InCount)
{
	SourceType = EPWInventoryDragSourceType::Inventory;
	SourceSlotIndex = InSourceSlotIndex;
	ItemData = InItemData;
	Count = InCount;
}

void UPWInventoryDragDropOperation::InitializeEquipmentPayload(int32 InSourceSlotIndex, UPWItemDataAsset* InItemData)
{
	SourceType = EPWInventoryDragSourceType::Equipment;
	SourceSlotIndex = InSourceSlotIndex;
	ItemData = InItemData;
	Count = InItemData ? 1 : 0;
}
