// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Components/PWPlayerInventoryLinkComponent.h"
#include "PWInventoryPanelWidget.generated.h"

class APWPlayerCharacter;
class UPWPlayerEquipmentComponent;
class UPWPlayerInventoryLinkComponent;
class UPWPlayerStatComponent;
class UPWEquipmentSlotWidget;
class UPWInventoryDropZoneWidget;
class UPWInventorySlotWidget;
class UProgressBar;
class UTextBlock;
class UUniformGridPanel;

UCLASS(Blueprintable)
class PALWORLD_API UPWInventoryPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|UI|Inventory")
	void InitializeWithInventoryComponent(UPWPlayerInventoryLinkComponent* InInventoryComponent);

	UFUNCTION(BlueprintCallable, Category = "Player|UI|Inventory")
	void InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter);

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	UPWPlayerInventoryLinkComponent* GetInventoryComponent() const { return BoundInventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	UPWPlayerEquipmentComponent* GetEquipmentComponent() const { return BoundEquipmentComponent; }

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	TArray<FPWInventorySlotView> GetSlotViews() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	int32 GetInventorySlotCount() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintPure, Category = "Player|UI|Inventory")
	float GetMaxCarryWeight() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Inventory", meta = (DisplayName = "On Inventory Changed"))
	void BP_OnInventoryChanged();

	// WBP_Inv에 같은 이름의 UniformGridPanel을 두면 인벤토리 슬롯을 C++에서 자동 생성한다.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UUniformGridPanel> Grid_Inventory = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Weapon0 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Weapon1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Weapon2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Weapon3 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Head = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Body = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Shield = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Glider = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_SphereModule = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Accessory0 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Accessory1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Food0 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Food1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Food2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWEquipmentSlotWidget> Slot_Food3 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWInventoryDropZoneWidget> DropZone_Drop = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UPWInventoryDropZoneWidget> DropZone_Destroy = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UTextBlock> Text_Weight = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UTextBlock> Text_HP = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UTextBlock> Text_Stamina = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UTextBlock> Text_Hunger = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UTextBlock> Text_Attack = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UTextBlock> Text_Defense = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UTextBlock> Text_WorkSpeed = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UProgressBar> Progress_HP = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UProgressBar> Progress_Stamina = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Inventory|Bind")
	TObjectPtr<UProgressBar> Progress_Hunger = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|UI|Inventory|Classes")
	TSubclassOf<UPWInventorySlotWidget> InventorySlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|UI|Inventory|Classes")
	TSubclassOf<UPWEquipmentSlotWidget> EquipmentSlotWidgetClass;

private:
	UFUNCTION()
	void HandleInventoryChanged();

	UFUNCTION()
	void HandleEquipmentChanged();

	UFUNCTION()
	void HandleSurvivalStatsChanged();

	void UnbindInventoryComponent();
	void UnbindEquipmentComponent();
	void UnbindStatComponent();
	void RebuildInventorySlots();
	void RebuildEquipmentSlots();
	void InitializeDropZones();
	void RefreshWeightText();
	void RefreshStatsText();
	void InitializeFixedEquipmentSlot(UPWEquipmentSlotWidget* SlotWidget, int32 SlotIndex);

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerInventoryLinkComponent> BoundInventoryComponent;

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerEquipmentComponent> BoundEquipmentComponent;

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerStatComponent> BoundStatComponent;
};
