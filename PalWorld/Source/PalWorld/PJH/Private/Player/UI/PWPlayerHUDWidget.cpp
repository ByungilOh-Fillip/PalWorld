// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWPlayerHUDWidget.h"

#include "Components/Widget.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "Player/UI/PWStaminaGaugeWidget.h"

void UPWPlayerHUDWidget::InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter)
{
	BoundPlayerCharacter = InPlayerCharacter;

	if (StaminaGauge)
	{
		StaminaGauge->InitializeWithStatComponent(InPlayerCharacter ? InPlayerCharacter->GetStatComponent() : nullptr);
	}

	SetCrosshairVisible(InPlayerCharacter && InPlayerCharacter->IsAiming());
}

void UPWPlayerHUDWidget::SetCrosshairVisible(bool bVisible)
{
	bIsCrosshairVisible = bVisible;
	BroadcastCrosshairVisibility();
}

void UPWPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BoundPlayerCharacter)
	{
		InitializeWithPlayerCharacter(BoundPlayerCharacter);
		return;
	}

	BroadcastCrosshairVisibility();
}

void UPWPlayerHUDWidget::BroadcastCrosshairVisibility()
{
	if (CrosshairRoot)
	{
		CrosshairRoot->SetVisibility(bIsCrosshairVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	BP_OnCrosshairVisibilityChanged(bIsCrosshairVisible);
}
