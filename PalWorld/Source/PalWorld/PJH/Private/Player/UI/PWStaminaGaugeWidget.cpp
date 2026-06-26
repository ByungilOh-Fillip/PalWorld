// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWStaminaGaugeWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Player/Components/PWPlayerStatComponent.h"

void UPWStaminaGaugeWidget::InitializeWithStatComponent(UPWPlayerStatComponent* InStatComponent)
{
	if (BoundStatComponent == InStatComponent)
	{
		if (BoundStatComponent)
		{
			SetStaminaRatio(BoundStatComponent->GetStaminaRatio());
		}
		return;
	}

	UnbindStatComponent();
	BoundStatComponent = InStatComponent;

	if (BoundStatComponent)
	{
		BoundStatComponent->OnStaminaChanged.AddDynamic(this, &UPWStaminaGaugeWidget::HandleStaminaChanged);
		SetStaminaRatio(BoundStatComponent->GetStaminaRatio());
	}
	else
	{
		SetStaminaRatio(PreviewStaminaRatio);
	}
}

void UPWStaminaGaugeWidget::SetStaminaRatio(float InStaminaRatio)
{
	CacheStaminaMaterial();

	const float SafeRatio = FMath::Clamp(InStaminaRatio, 0.f, 1.f);
	UpdateGaugeVisibility(SafeRatio);

	if (StaminaMaterial)
	{
		StaminaMaterial->SetScalarParameterValue(FillAmountParameterName, SafeRatio);
	}

	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(SafeRatio);
	}
}

void UPWStaminaGaugeWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetStaminaRatio(BoundStatComponent ? BoundStatComponent->GetStaminaRatio() : PreviewStaminaRatio);
}

void UPWStaminaGaugeWidget::NativeDestruct()
{
	UnbindStatComponent();

	Super::NativeDestruct();
}

void UPWStaminaGaugeWidget::HandleStaminaChanged(float CurrentStamina, float MaxStamina, float StaminaRatio)
{
	SetStaminaRatio(StaminaRatio);
}

void UPWStaminaGaugeWidget::UnbindStatComponent()
{
	if (BoundStatComponent)
	{
		BoundStatComponent->OnStaminaChanged.RemoveDynamic(this, &UPWStaminaGaugeWidget::HandleStaminaChanged);
		BoundStatComponent = nullptr;
	}
}

void UPWStaminaGaugeWidget::CacheStaminaMaterial()
{
	if (StaminaMaterial || !StaminaFillImage)
	{
		return;
	}

	UMaterialInterface* SourceMaterial = Cast<UMaterialInterface>(StaminaFillImage->GetBrush().GetResourceObject());
	if (!SourceMaterial)
	{
		return;
	}

	StaminaMaterial = UMaterialInstanceDynamic::Create(SourceMaterial, this);
	StaminaFillImage->SetBrushFromMaterial(StaminaMaterial);
}

void UPWStaminaGaugeWidget::UpdateGaugeVisibility(float StaminaRatio)
{
	if (IsDesignTime())
	{
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		return;
	}

	if (bHideWhenFull && StaminaRatio >= FullHiddenThreshold)
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}
