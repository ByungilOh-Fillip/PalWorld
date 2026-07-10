// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWPlayerHUDWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Player/Components/PWPlayerCaptureComponent.h"
#include "Player/Components/PWPlayerStatComponent.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "Player/UI/PWCaptureAimWidget.h"
#include "Player/UI/PWInventoryPanelWidget.h"
#include "Player/UI/PWPalPartyPanelWidget.h"
#include "Player/UI/PWStaminaGaugeWidget.h"

void UPWPlayerHUDWidget::InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter)
{
	BoundPlayerCharacter = InPlayerCharacter;
	BindStatComponent(InPlayerCharacter ? InPlayerCharacter->GetStatComponent() : nullptr);
	BindCaptureComponent();

	if (StaminaGauge)
	{
		StaminaGauge->InitializeWithStatComponent(BoundStatComponent);
	}

	if (UPWInventoryPanelWidget* ActiveInventoryPanel = GetOrCreateInventoryPanel())
	{
		ActiveInventoryPanel->InitializeWithPlayerCharacter(InPlayerCharacter);
	}

	if (CaptureAim)
	{
		CaptureAim->InitializeWithPlayerCharacter(InPlayerCharacter);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWCaptureAim] WBP_PlayerHUD does not have a bound CaptureAim widget."));
	}

	if (PalPartyPanel)
	{
		PalPartyPanel->InitializeWithPlayerCharacter(InPlayerCharacter);
	}

	SetCrosshairVisible(InPlayerCharacter && InPlayerCharacter->IsAiming());
	RefreshSurvivalStats();
	BroadcastCaptureAimChanged();
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

bool UPWPlayerHUDWidget::IsCaptureAimVisible() const
{
	return BoundCaptureComponent && BoundCaptureComponent->IsCaptureAimVisible();
}

bool UPWPlayerHUDWidget::HasCaptureAimTarget() const
{
	return BoundCaptureComponent && BoundCaptureComponent->HasCaptureAimTarget();
}

float UPWPlayerHUDWidget::GetCaptureAimChance() const
{
	return BoundCaptureComponent ? BoundCaptureComponent->GetCaptureAimChance() : 0.f;
}

int32 UPWPlayerHUDWidget::GetCaptureAimChancePercent() const
{
	return BoundCaptureComponent ? BoundCaptureComponent->GetCaptureAimChancePercent() : 0;
}

FText UPWPlayerHUDWidget::GetCaptureAimTargetNameText() const
{
	return BoundCaptureComponent ? BoundCaptureComponent->GetCaptureAimTargetNameText() : FText::GetEmpty();
}

int32 UPWPlayerHUDWidget::GetCaptureSphereCount() const
{
	return BoundCaptureComponent ? BoundCaptureComponent->GetThrowableCaptureSphereCount() : 0;
}

void UPWPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BoundPlayerCharacter)
	{
		InitializeWithPlayerCharacter(BoundPlayerCharacter);
		return;
	}

	RefreshSurvivalStats();
	BroadcastCrosshairVisibility();
	BroadcastInventoryVisibility();
}

void UPWPlayerHUDWidget::NativeDestruct()
{
	UnbindStatComponent();
	UnbindCaptureComponent();

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

void UPWPlayerHUDWidget::HandleSurvivalStatsChanged()
{
	RefreshSurvivalStats();
}

void UPWPlayerHUDWidget::BindStatComponent(UPWPlayerStatComponent* InStatComponent)
{
	if (BoundStatComponent == InStatComponent)
	{
		return;
	}

	UnbindStatComponent();
	BoundStatComponent = InStatComponent;
	if (BoundStatComponent)
	{
		BoundStatComponent->OnSurvivalStatsChanged.AddUniqueDynamic(this, &UPWPlayerHUDWidget::HandleSurvivalStatsChanged);
	}
}

void UPWPlayerHUDWidget::BindCaptureComponent()
{
	UPWPlayerCaptureComponent* NewCaptureComponent = BoundPlayerCharacter ? BoundPlayerCharacter->GetCaptureComponent() : nullptr;
	if (BoundCaptureComponent == NewCaptureComponent)
	{
		return;
	}

	UnbindCaptureComponent();
	BoundCaptureComponent = NewCaptureComponent;
	if (BoundCaptureComponent)
	{
		BoundCaptureComponent->OnCaptureAimInfoChanged.AddUniqueDynamic(this, &UPWPlayerHUDWidget::HandleCaptureAimInfoChanged);
	}
}

void UPWPlayerHUDWidget::UnbindCaptureComponent()
{
	if (BoundCaptureComponent)
	{
		BoundCaptureComponent->OnCaptureAimInfoChanged.RemoveDynamic(this, &UPWPlayerHUDWidget::HandleCaptureAimInfoChanged);
		BoundCaptureComponent = nullptr;
	}
}

void UPWPlayerHUDWidget::UnbindStatComponent()
{
	if (BoundStatComponent)
	{
		BoundStatComponent->OnSurvivalStatsChanged.RemoveDynamic(this, &UPWPlayerHUDWidget::HandleSurvivalStatsChanged);
		BoundStatComponent = nullptr;
	}
}

void UPWPlayerHUDWidget::BroadcastCaptureAimChanged()
{
	BP_OnCaptureAimChanged();
}

void UPWPlayerHUDWidget::HandleCaptureAimInfoChanged()
{
	BroadcastCaptureAimChanged();
}

void UPWPlayerHUDWidget::RefreshSurvivalStats()
{
	const float HealthRatio = BoundStatComponent ? BoundStatComponent->GetHealthRatio() : 1.f;
	const float ShieldRatio = BoundStatComponent ? BoundStatComponent->GetShieldRatio() : 1.f;
	const float HungerRatio = BoundStatComponent ? BoundStatComponent->GetHungerRatio() : 1.f;
	const bool bHasShieldCapacity = BoundStatComponent && BoundStatComponent->HasShieldCapacity();

	if (Progress_Health)
	{
		Progress_Health->SetPercent(HealthRatio);
	}

	if (ShieldRoot)
	{
		ShieldRoot->SetVisibility(bHasShieldCapacity ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Progress_Shield)
	{
		Progress_Shield->SetPercent(ShieldRatio);
		Progress_Shield->SetVisibility(bHasShieldCapacity ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Progress_Hunger)
	{
		Progress_Hunger->SetPercent(HungerRatio);
	}

	if (Text_Health)
	{
		Text_Health->SetText(BoundStatComponent
			? FText::Format(
				NSLOCTEXT("PWPlayerHUD", "HealthFormat", "{0} / {1}"),
				FText::AsNumber(FMath::RoundToInt(BoundStatComponent->GetCurrentHealth())),
				FText::AsNumber(FMath::RoundToInt(BoundStatComponent->GetMaxHealth())))
			: FText::GetEmpty());
	}

	if (Text_Shield)
	{
		Text_Shield->SetText(BoundStatComponent
			? FText::Format(
				NSLOCTEXT("PWPlayerHUD", "ShieldFormat", "{0} / {1}"),
				FText::AsNumber(FMath::RoundToInt(BoundStatComponent->GetCurrentShield())),
				FText::AsNumber(FMath::RoundToInt(BoundStatComponent->GetMaxShield())))
			: FText::GetEmpty());
		Text_Shield->SetVisibility(bHasShieldCapacity ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Text_Hunger)
	{
		Text_Hunger->SetText(BoundStatComponent
			? FText::Format(
				NSLOCTEXT("PWPlayerHUD", "HungerFormat", "{0} / {1}"),
				FText::AsNumber(FMath::RoundToInt(BoundStatComponent->GetCurrentHunger())),
				FText::AsNumber(FMath::RoundToInt(BoundStatComponent->GetMaxHunger())))
			: FText::GetEmpty());
	}

	BP_OnSurvivalStatsChanged();
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
