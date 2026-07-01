// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWPlayerHUDWidget.h"

#include "Components/Widget.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "Player/UI/PWInventoryPanelWidget.h"
#include "Player/UI/PWStaminaGaugeWidget.h"

void UPWPlayerHUDWidget::InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter)
{
	BoundPlayerCharacter = InPlayerCharacter;

	if (StaminaGauge)
	{
		StaminaGauge->InitializeWithStatComponent(InPlayerCharacter ? InPlayerCharacter->GetStatComponent() : nullptr);
	}

	if (UPWInventoryPanelWidget* ActiveInventoryPanel = GetOrCreateInventoryPanel())
	{
		ActiveInventoryPanel->InitializeWithPlayerCharacter(InPlayerCharacter);
	}

	SetCrosshairVisible(InPlayerCharacter && InPlayerCharacter->IsAiming());
	BroadcastInventoryVisibility();
}

void UPWPlayerHUDWidget::SetCrosshairVisible(bool bVisible)
{
	bIsCrosshairVisible = bVisible;
	BroadcastCrosshairVisibility();
}

void UPWPlayerHUDWidget::SetInventoryVisible(bool bVisible)
{
	bIsInventoryVisible = bVisible;
	BroadcastInventoryVisibility();
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
	BroadcastInventoryVisibility();
}

void UPWPlayerHUDWidget::NativeDestruct()
{
	if (CreatedInventoryPanel)
	{
		CreatedInventoryPanel->RemoveFromParent();
		CreatedInventoryPanel = nullptr;
	}

	Super::NativeDestruct();
}

void UPWPlayerHUDWidget::BroadcastCrosshairVisibility()
{
	if (CrosshairRoot)
	{
		CrosshairRoot->SetVisibility(bIsCrosshairVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	BP_OnCrosshairVisibilityChanged(bIsCrosshairVisible);
}

void UPWPlayerHUDWidget::BroadcastInventoryVisibility()
{
	if (UPWInventoryPanelWidget* ActiveInventoryPanel = GetOrCreateInventoryPanel())
	{
		ActiveInventoryPanel->SetVisibility(bIsInventoryVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	BP_OnInventoryVisibilityChanged(bIsInventoryVisible);
}

UPWInventoryPanelWidget* UPWPlayerHUDWidget::GetOrCreateInventoryPanel()
{
	if (InventoryPanel)
	{
		return InventoryPanel;
	}

	if (!CreatedInventoryPanel)
	{
		TSubclassOf<UPWInventoryPanelWidget> PanelClass = InventoryPanelWidgetClass;
		if (!PanelClass)
		{
			PanelClass = UPWInventoryPanelWidget::StaticClass();
		}

		CreatedInventoryPanel = CreateWidget<UPWInventoryPanelWidget>(GetOwningPlayer(), PanelClass);
		if (CreatedInventoryPanel)
		{
			CreatedInventoryPanel->AddToViewport(500);
			CreatedInventoryPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PWInventory] Failed to create inventory panel. Class=%s Owner=%s"),
				*GetNameSafe(PanelClass.Get()),
				*GetNameSafe(GetOwningPlayer()));
		}
	}

	return CreatedInventoryPanel;
}
