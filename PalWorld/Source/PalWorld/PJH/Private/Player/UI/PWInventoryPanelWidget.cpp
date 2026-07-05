// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWInventoryPanelWidget.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Player/Components/PWPlayerEquipmentComponent.h"
#include "Player/Components/PWPlayerStatComponent.h"
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

	UnbindStatComponent();
	BoundStatComponent = InPlayerCharacter ? InPlayerCharacter->GetStatComponent() : nullptr;
	if (BoundStatComponent)
	{
		BoundStatComponent->OnSurvivalStatsChanged.AddUniqueDynamic(this, &UPWInventoryPanelWidget::HandleSurvivalStatsChanged);
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
	UnbindStatComponent();
	Super::NativeDestruct();
}

void UPWInventoryPanelWidget::HandleInventoryChanged()
{
	InitializeDropZones();
	RebuildInventorySlots();
	RebuildEquipmentSlots();
	RefreshWeightText();
	RefreshStatsText();
	BP_OnInventoryChanged();
}

void UPWInventoryPanelWidget::HandleEquipmentChanged()
{
	RebuildInventorySlots();
	RebuildEquipmentSlots();
	RefreshWeightText();
	RefreshStatsText();
	BP_OnInventoryChanged();
}

void UPWInventoryPanelWidget::HandleSurvivalStatsChanged()
{
	RefreshStatsText();
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

void UPWInventoryPanelWidget::UnbindStatComponent()
{
	if (BoundStatComponent)
	{
		BoundStatComponent->OnSurvivalStatsChanged.RemoveDynamic(this, &UPWInventoryPanelWidget::HandleSurvivalStatsChanged);
		BoundStatComponent = nullptr;
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

void UPWInventoryPanelWidget::RefreshStatsText()
{
	if (Progress_HP)
	{
		Progress_HP->SetPercent(BoundStatComponent ? BoundStatComponent->GetHealthRatio() : 1.f);
	}

	if (Progress_Stamina)
	{
		Progress_Stamina->SetPercent(BoundStatComponent ? BoundStatComponent->GetStaminaRatio() : 1.f);
	}

	if (Progress_Hunger)
	{
		Progress_Hunger->SetPercent(BoundStatComponent ? BoundStatComponent->GetHungerRatio() : 1.f);
	}

	if (Text_HP)
	{
		Text_HP->SetText(BoundStatComponent
			? FText::Format(
				NSLOCTEXT("PWInventory", "HPFormat", "{0} / {1}"),
				FText::AsNumber(FMath::RoundToInt(BoundStatComponent->GetCurrentHealth())),
				FText::AsNumber(FMath::RoundToInt(BoundStatComponent->GetMaxHealth())))
			: FText::GetEmpty());
	}

	if (Text_Stamina)
	{
		Text_Stamina->SetText(BoundStatComponent
			? FText::Format(
				NSLOCTEXT("PWInventory", "StaminaFormat", "{0} / {1}"),
				FText::AsNumber(FMath::RoundToInt(BoundStatComponent->GetCurrentStamina())),
				FText::AsNumber(FMath::RoundToInt(BoundStatComponent->GetMaxStamina())))
			: FText::GetEmpty());
	}

	if (Text_Hunger)
	{
		Text_Hunger->SetText(BoundStatComponent
			? FText::Format(
				NSLOCTEXT("PWInventory", "HungerFormat", "{0} / {1}"),
				FText::AsNumber(FMath::RoundToInt(BoundStatComponent->GetCurrentHunger())),
				FText::AsNumber(FMath::RoundToInt(BoundStatComponent->GetMaxHunger())))
			: FText::GetEmpty());
	}

	if (Text_Attack)
	{
		Text_Attack->SetText(BoundStatComponent ? FText::AsNumber(FMath::RoundToInt(BoundStatComponent->Attack)) : FText::GetEmpty());
	}

	if (Text_Defense)
	{
		Text_Defense->SetText(BoundStatComponent ? FText::AsNumber(FMath::RoundToInt(BoundStatComponent->Defense)) : FText::GetEmpty());
	}

	if (Text_WorkSpeed)
	{
		Text_WorkSpeed->SetText(BoundStatComponent ? FText::AsNumber(FMath::RoundToInt(BoundStatComponent->WorkSpeed)) : FText::GetEmpty());
	}
}

void UPWInventoryPanelWidget::InitializeFixedEquipmentSlot(UPWEquipmentSlotWidget* SlotWidget, int32 SlotIndex)
{
	if (SlotWidget)
	{
		SlotWidget->InitializeEquipmentSlot(BoundEquipmentComponent, BoundInventoryComponent, SlotIndex);
	}
}
