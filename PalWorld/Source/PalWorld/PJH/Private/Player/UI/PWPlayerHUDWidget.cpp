// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWPlayerHUDWidget.h"

#include "Player/Core/PWPlayerCharacter.h"
#include "Player/UI/PWStaminaGaugeWidget.h"

void UPWPlayerHUDWidget::InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter)
{
	BoundPlayerCharacter = InPlayerCharacter;

	if (StaminaGauge)
	{
		StaminaGauge->InitializeWithStatComponent(InPlayerCharacter ? InPlayerCharacter->GetStatComponent() : nullptr);
	}
}

void UPWPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BoundPlayerCharacter)
	{
		InitializeWithPlayerCharacter(BoundPlayerCharacter);
	}
}
