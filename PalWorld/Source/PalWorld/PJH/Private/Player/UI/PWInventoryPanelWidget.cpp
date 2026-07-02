// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWInventoryPanelWidget.h"

#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Player/Components/PWPlayerEquipmentComponent.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "Player/Data/PWItemDataAsset.h"
#include "Player/UI/PWEquipmentSlotWidget.h"
#include "Player/UI/PWInventoryDropZoneWidget.h"
#include "Player/UI/PWInventorySlotWidget.h"

void UPWInventoryPanelWidget::InitializeWithInventoryComponent(UPWPlayerInventoryLinkComponent* InInventoryComponent)
{
	if (BoundInventoryComponent == InInventoryComponent)
	{
		HandleInventoryChanged();
		return;
	}

	UnbindInventoryComponent();
	BoundInventoryComponent = InInventoryComponent;

	if (BoundInventoryComponent)
	{
		BoundInventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &UPWInventoryPanelWidget::HandleInventoryChanged);
	}

	HandleInventoryChanged();
}

void UPWInventoryPanelWidget::InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter)
{
	UnbindEquipmentComponent();
	BoundEquipmentComponent = InPlayerCharacter ? InPlayerCharacter->GetEquipmentComponent() : nullptr;
	if (BoundEquipmentComponent)
	{
		BoundEquipmentComponent->OnEquipmentChanged.AddUniqueDynamic(this, &UPWInventoryPanelWidget::HandleEquipmentChanged);
	}

	InitializeWithInventoryComponent(InPlayerCharacter ? InPlayerCharacter->GetInventoryLinkComponent() : nullptr);
}

TArray<FPWInventorySlotView> UPWInventoryPanelWidget::GetSlotViews() const
{
	return BoundInventoryComponent ? BoundInventoryComponent->GetSlotViews() : TArray<FPWInventorySlotView>();
}

int32 UPWInventoryPanelWidget::GetInventorySlotCount() const
{
	return BoundInventoryComponent ? BoundInventoryComponent->GetInventorySlotCount() : 0;
}

float UPWInventoryPanelWidget::GetCurrentWeight() const
{
	return BoundInventoryComponent ? BoundInventoryComponent->GetCurrentWeight() : 0.f;
}

float UPWInventoryPanelWidget::GetMaxCarryWeight() const
{
	return BoundInventoryComponent ? BoundInventoryComponent->GetMaxCarryWeight() : 0.f;
}

void UPWInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitializeDropZones();
	HandleInventoryChanged();
}

void UPWInventoryPanelWidget::NativeDestruct()
{
	UnbindInventoryComponent();
	UnbindEquipmentComponent();
	Super::NativeDestruct();
}

void UPWInventoryPanelWidget::HandleInventoryChanged()
{
	InitializeDropZones();
	RebuildInventorySlots();
	RebuildEquipmentSlots();
	RefreshWeightText();
	BP_OnInventoryChanged();
}

void UPWInventoryPanelWidget::HandleEquipmentChanged()
{
	RebuildInventorySlots();
	RebuildEquipmentSlots();
	RefreshWeightText();
	BP_OnInventoryChanged();
}

void UPWInventoryPanelWidget::UnbindInventoryComponent()
{
	if (BoundInventoryComponent)
	{
		BoundInventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UPWInventoryPanelWidget::HandleInventoryChanged);
		BoundInventoryComponent = nullptr;
	}
}

void UPWInventoryPanelWidget::UnbindEquipmentComponent()
{
	if (BoundEquipmentComponent)
	{
		BoundEquipmentComponent->OnEquipmentChanged.RemoveDynamic(this, &UPWInventoryPanelWidget::HandleEquipmentChanged);
		BoundEquipmentComponent = nullptr;
	}
}

void UPWInventoryPanelWidget::RebuildInventorySlots()
{
	UClass* ResolvedInventorySlotClass = InventorySlotWidgetClass ? InventorySlotWidgetClass.Get() : UPWInventorySlotWidget::StaticClass();
	if (!Grid_Inventory || !ResolvedInventorySlotClass)
	{
		return;
	}

	Grid_Inventory->ClearChildren();

	constexpr int32 InventoryColumnCount = 6;
	const TArray<FPWInventorySlotView> SlotViews = GetSlotViews();
	for (const FPWInventorySlotView& SlotView : SlotViews)
	{
		UPWInventorySlotWidget* SlotWidget = CreateWidget<UPWInventorySlotWidget>(GetOwningPlayer(), ResolvedInventorySlotClass);
		if (!SlotWidget)
		{
			continue;
		}

		SlotWidget->InitializeInventorySlot(BoundInventoryComponent, BoundEquipmentComponent, SlotView);

		UUniformGridSlot* GridSlot = Grid_Inventory->AddChildToUniformGrid(
			SlotWidget,
			SlotView.SlotIndex / InventoryColumnCount,
			SlotView.SlotIndex % InventoryColumnCount);
		if (GridSlot)
		{
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
}

void UPWInventoryPanelWidget::RebuildEquipmentSlots()
{
	InitializeFixedEquipmentSlot(Slot_Weapon0, 0);
	InitializeFixedEquipmentSlot(Slot_Weapon1, 1);
	InitializeFixedEquipmentSlot(Slot_Weapon2, 2);
	InitializeFixedEquipmentSlot(Slot_Weapon3, 3);
	InitializeFixedEquipmentSlot(Slot_Head, UPWPlayerEquipmentComponent::HeadSlotIndex);
	InitializeFixedEquipmentSlot(Slot_Body, UPWPlayerEquipmentComponent::BodySlotIndex);
	InitializeFixedEquipmentSlot(Slot_Shield, UPWPlayerEquipmentComponent::ShieldSlotIndex);
	InitializeFixedEquipmentSlot(Slot_Glider, UPWPlayerEquipmentComponent::GliderSlotIndex);
	InitializeFixedEquipmentSlot(Slot_SphereModule, UPWPlayerEquipmentComponent::SphereModuleSlotIndex);
	InitializeFixedEquipmentSlot(Slot_Accessory0, UPWPlayerEquipmentComponent::AccessorySlotStartIndex);
	InitializeFixedEquipmentSlot(Slot_Accessory1, UPWPlayerEquipmentComponent::AccessorySlotStartIndex + 1);
	InitializeFixedEquipmentSlot(Slot_Food0, UPWPlayerEquipmentComponent::FoodSlotStartIndex);
	InitializeFixedEquipmentSlot(Slot_Food1, UPWPlayerEquipmentComponent::FoodSlotStartIndex + 1);
	InitializeFixedEquipmentSlot(Slot_Food2, UPWPlayerEquipmentComponent::FoodSlotStartIndex + 2);
	InitializeFixedEquipmentSlot(Slot_Food3, UPWPlayerEquipmentComponent::FoodSlotStartIndex + 3);

}

void UPWInventoryPanelWidget::InitializeDropZones()
{
	if (DropZone_Drop)
	{
		DropZone_Drop->InitializeDropZone(BoundInventoryComponent, BoundEquipmentComponent);
	}

	if (DropZone_Destroy)
	{
		DropZone_Destroy->InitializeDropZone(BoundInventoryComponent, BoundEquipmentComponent);
	}
}

void UPWInventoryPanelWidget::RefreshWeightText()
{
	if (!Text_Weight)
	{
		return;
	}

	Text_Weight->SetText(FText::Format(
		NSLOCTEXT("PWInventory", "WeightFormat", "{0} / {1}"),
		FText::AsNumber(GetCurrentWeight()),
		FText::AsNumber(GetMaxCarryWeight())));
}

void UPWInventoryPanelWidget::InitializeFixedEquipmentSlot(UPWEquipmentSlotWidget* SlotWidget, int32 SlotIndex)
{
	if (SlotWidget)
	{
		SlotWidget->InitializeEquipmentSlot(BoundEquipmentComponent, BoundInventoryComponent, SlotIndex);
	}
}
