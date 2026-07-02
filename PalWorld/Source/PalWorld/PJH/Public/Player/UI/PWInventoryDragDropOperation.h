// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "PWInventoryDragDropOperation.generated.h"

class UPWItemDataAsset;

UENUM(BlueprintType)
enum class EPWInventoryDragSourceType : uint8
{
	None,
	Inventory,
	Equipment
};

UCLASS(BlueprintType)
class PALWORLD_API UPWInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|Inventory|DragDrop")
	void InitializeInventoryPayload(int32 InSourceSlotIndex, UPWItemDataAsset* InItemData, int32 InCount);

	UFUNCTION(BlueprintCallable, Category = "Player|Inventory|DragDrop")
	void InitializeEquipmentPayload(int32 InSourceSlotIndex, UPWItemDataAsset* InItemData);

	UFUNCTION(BlueprintPure, Category = "Player|Inventory|DragDrop")
	bool IsFromInventory() const { return SourceType == EPWInventoryDragSourceType::Inventory; }

	UFUNCTION(BlueprintPure, Category = "Player|Inventory|DragDrop")
	bool IsFromEquipment() const { return SourceType == EPWInventoryDragSourceType::Equipment; }

	UPROPERTY(BlueprintReadOnly, Category = "Player|Inventory|DragDrop")
	EPWInventoryDragSourceType SourceType = EPWInventoryDragSourceType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Inventory|DragDrop")
	int32 SourceSlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Inventory|DragDrop")
	int32 Count = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Inventory|DragDrop")
	TObjectPtr<UPWItemDataAsset> ItemData = nullptr;
};
