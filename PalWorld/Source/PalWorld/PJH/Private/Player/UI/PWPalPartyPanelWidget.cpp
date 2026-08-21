// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWPalPartyPanelWidget.h"

#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "Player/UI/PWPalPartySlotWidget.h"

void UPWPalPartyPanelWidget::InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter)
{
	InitializeWithPalStorageComponent(InPlayerCharacter ? InPlayerCharacter->GetPalStorageComponent() : nullptr);
}

void UPWPalPartyPanelWidget::InitializeWithPalStorageComponent(UPWPlayerPalStorageComponent* InPalStorageComponent)
{
	if (BoundPalStorageComponent == InPalStorageComponent)
	{
		RefreshPalParty();
		return;
	}

	UnbindPalStorageComponent();
	BoundPalStorageComponent = InPalStorageComponent;

	if (BoundPalStorageComponent)
	{
		BoundPalStorageComponent->OnCapturedPalsChanged.AddUniqueDynamic(this, &UPWPalPartyPanelWidget::HandleCapturedPalsChanged);
	}

	RefreshPalParty();
}

void UPWPalPartyPanelWidget::RefreshPalParty()
{
	const TArray<FPWCapturedPalRecord> CapturedPals = GetCapturedPals();

	if (HasFixedPartySlots())
	{
		RefreshFixedSlots(CapturedPals);
	}
	else
	{
		RebuildGeneratedSlots(CapturedPals);
	}

	if (Text_Empty)
	{
		Text_Empty->SetVisibility(CapturedPals.IsEmpty() ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Text_Count)
	{
		Text_Count->SetText(FText::Format(
			NSLOCTEXT("PWPalParty", "PalCountFormat", "{0}"),
			FText::AsNumber(CapturedPals.Num())));
	}

	BP_OnPalPartyChanged();
}

TArray<FPWCapturedPalRecord> UPWPalPartyPanelWidget::GetCapturedPals() const
{
	TArray<FPWCapturedPalRecord> CapturedPals;
	if (BoundPalStorageComponent)
	{
		BoundPalStorageComponent->GetCapturedPals(CapturedPals);
	}
	return CapturedPals;
}

void UPWPalPartyPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshPalParty();
}

void UPWPalPartyPanelWidget::NativeDestruct()
{
	UnbindPalStorageComponent();
	Super::NativeDestruct();
}

void UPWPalPartyPanelWidget::HandleCapturedPalsChanged()
{
	RefreshPalParty();
}

void UPWPalPartyPanelWidget::UnbindPalStorageComponent()
{
	if (BoundPalStorageComponent)
	{
		BoundPalStorageComponent->OnCapturedPalsChanged.RemoveDynamic(this, &UPWPalPartyPanelWidget::HandleCapturedPalsChanged);
		BoundPalStorageComponent = nullptr;
	}
}

void UPWPalPartyPanelWidget::RefreshFixedSlots(const TArray<FPWCapturedPalRecord>& CapturedPals)
{
	TArray<UPWPalPartySlotWidget*> FixedSlots;
	FixedSlots.Reserve(5);
	FixedSlots.Add(Slot_Party0);
	FixedSlots.Add(Slot_Party1);
	FixedSlots.Add(Slot_Party2);
	FixedSlots.Add(Slot_Party3);
	FixedSlots.Add(Slot_Party4);

	for (int32 SlotIndex = 0; SlotIndex < FixedSlots.Num(); ++SlotIndex)
	{
		UPWPalPartySlotWidget* SlotWidget = FixedSlots[SlotIndex];
		if (!SlotWidget)
		{
			continue;
		}

		const bool bHasPal = CapturedPals.IsValidIndex(SlotIndex);
		SlotWidget->SetPalRecord(bHasPal ? CapturedPals[SlotIndex] : FPWCapturedPalRecord(), bHasPal);
	}
}

void UPWPalPartyPanelWidget::RebuildGeneratedSlots(const TArray<FPWCapturedPalRecord>& CapturedPals)
{
	GeneratedSlotWidgets.Reset();

	if (!Panel_PartySlots)
	{
		return;
	}

	Panel_PartySlots->ClearChildren();

	TSubclassOf<UPWPalPartySlotWidget> SlotClass = PartySlotWidgetClass;
	if (!SlotClass)
	{
		SlotClass = UPWPalPartySlotWidget::StaticClass();
	}

	for (const FPWCapturedPalRecord& CapturedPal : CapturedPals)
	{
		UPWPalPartySlotWidget* SlotWidget = CreateWidget<UPWPalPartySlotWidget>(GetOwningPlayer(), SlotClass);
		if (!SlotWidget)
		{
			continue;
		}

		SlotWidget->SetPalRecord(CapturedPal, true);
		Panel_PartySlots->AddChild(SlotWidget);
		GeneratedSlotWidgets.Add(SlotWidget);
	}
}

bool UPWPalPartyPanelWidget::HasFixedPartySlots() const
{
	return Slot_Party0 || Slot_Party1 || Slot_Party2 || Slot_Party3 || Slot_Party4;
}
