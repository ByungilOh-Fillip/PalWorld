// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWCaptureProgressWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UPWCaptureProgressWidget::UPWCaptureProgressWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ProgressMaterialFinder(
		TEXT("/Game/PJH/UI/Materials/MI_UI_CaptureRing_Procedural.MI_UI_CaptureRing_Procedural"));
	if (ProgressMaterialFinder.Succeeded())
	{
		ProgressMaterial = ProgressMaterialFinder.Object;
	}
}

void UPWCaptureProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!Image_CaptureProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWCaptureProgress] WBP is missing Image_CaptureProgress."));
	}
	if (!Text_CaptureChance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWCaptureProgress] WBP is missing Text_CaptureChance."));
	}

	InitializeProgressMaterial();
	SetCaptureProgress(0.f, false);
}

void UPWCaptureProgressWidget::SetCaptureProgress(float DisplayChance, bool bCapturing)
{
	const float ClampedChance = FMath::Clamp(DisplayChance, 0.f, 1.f);

	if (Text_CaptureChance)
	{
		Text_CaptureChance->SetText(FText::Format(
			NSLOCTEXT("PWCaptureProgress", "CaptureChancePercent", "{0}%"),
			FText::AsNumber(FMath::RoundToInt(ClampedChance * 100.f))));
	}

	if (!ProgressMaterialInstance)
	{
		return;
	}

	const FLinearColor ProgressColor = FLinearColor::LerpUsingHSV(
		FLinearColor(1.f, 0.42f, 0.02f, 1.f),
		FLinearColor(0.05f, 1.f, 0.72f, 1.f),
		ClampedChance);

	ProgressMaterialInstance->SetScalarParameterValue(TEXT("Progress"), ClampedChance);
	ProgressMaterialInstance->SetScalarParameterValue(TEXT("Chance"), ClampedChance);
	ProgressMaterialInstance->SetScalarParameterValue(TEXT("IsCapturing"), bCapturing ? 1.f : 0.f);
	ProgressMaterialInstance->SetVectorParameterValue(TEXT("ProgressColor"), ProgressColor);
}

void UPWCaptureProgressWidget::SetCaptureVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UPWCaptureProgressWidget::InitializeProgressMaterial()
{
	if (!Image_CaptureProgress)
	{
		return;
	}

	if (!ProgressMaterial)
	{
		ProgressMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Game/PJH/UI/Materials/MI_UI_CaptureRing_Procedural.MI_UI_CaptureRing_Procedural"));
	}

	if (ProgressMaterial)
	{
		Image_CaptureProgress->SetBrushFromMaterial(ProgressMaterial);
		Image_CaptureProgress->SetColorAndOpacity(FLinearColor::White);
		ProgressMaterialInstance = Image_CaptureProgress->GetDynamicMaterial();
	}
}
