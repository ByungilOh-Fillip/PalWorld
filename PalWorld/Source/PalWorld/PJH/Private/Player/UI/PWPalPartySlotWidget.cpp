// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWPalPartySlotWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

void UPWPalPartySlotWidget::SetPalRecord(const FPWCapturedPalRecord& InPalRecord, bool bInHasPal)
{
	PalRecord = InPalRecord;
	bHasPal = bInHasPal;

	RefreshBoundWidgets();
	BP_OnPalSlotUpdated();
}

void UPWPalPartySlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	RefreshBoundWidgets();
}

void UPWPalPartySlotWidget::RefreshBoundWidgets()
{
	if (Panel_PalRoot)
	{
		Panel_PalRoot->SetVisibility(bHasPal ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Text_Name)
	{
		Text_Name->SetText(bHasPal ? FText::FromString(PalRecord.DisplayName) : FText::GetEmpty());
		Text_Name->SetVisibility(bHasPal ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Text_Level)
	{
		Text_Level->SetText(bHasPal
			? FText::Format(NSLOCTEXT("PWPalParty", "LevelFormat", "Lv. {0}"), FText::AsNumber(PalRecord.Level))
			: FText::GetEmpty());
		Text_Level->SetVisibility(bHasPal ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	const bool bHasHealth = bHasPal && PalRecord.MaxHP > 0.f;
	const float HealthRatio = bHasHealth ? FMath::Clamp(PalRecord.CurrentHP / PalRecord.MaxHP, 0.f, 1.f) : 0.f;

	if (Progress_HP)
	{
		Progress_HP->SetPercent(HealthRatio);
		Progress_HP->SetVisibility(bHasHealth ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Text_HP)
	{
		Text_HP->SetText(bHasHealth
			? FText::Format(
				NSLOCTEXT("PWPalParty", "HPFormat", "{0} / {1}"),
				FText::AsNumber(FMath::RoundToInt(PalRecord.CurrentHP)),
				FText::AsNumber(FMath::RoundToInt(PalRecord.MaxHP)))
			: FText::GetEmpty());
		Text_HP->SetVisibility(bHasHealth ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
