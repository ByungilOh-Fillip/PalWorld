// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PWInventoryDropZoneWidget.generated.h"

class UPWPlayerEquipmentComponent;
class UPWPlayerInventoryLinkComponent;

UENUM(BlueprintType)
enum class EPWInventoryDropZoneType : uint8
{
	Drop,
	Destroy
};

UCLASS(Blueprintable)
class PALWORLD_API UPWInventoryDropZoneWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|UI|Inventory")
	void InitializeDropZone(UPWPlayerInventoryLinkComponent* InInventoryComponent, UPWPlayerEquipmentComponent* InEquipmentComponent);

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|UI|Inventory")
	EPWInventoryDropZoneType ZoneType = EPWInventoryDropZoneType::Drop;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerInventoryLinkComponent> InventoryComponent;

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerEquipmentComponent> EquipmentComponent;
};
