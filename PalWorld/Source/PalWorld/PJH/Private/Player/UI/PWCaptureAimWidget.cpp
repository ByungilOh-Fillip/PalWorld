// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWCaptureAimWidget.h"

#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Player/Components/PWPlayerCaptureComponent.h"
#include "Player/Core/PWPlayerCharacter.h"

void UPWCaptureAimWidget::InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter)
{
	BoundPlayerCharacter = InPlayerCharacter;
	BindCaptureComponent(InPlayerCharacter ? InPlayerCharacter->GetCaptureComponent() : nullptr);
	RefreshCaptureAim();
}

void UPWCaptureAimWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!Root_CaptureAim)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWCaptureAim] WBP_CaptureAim is missing Root_CaptureAim."));
	}
	if (!Panel_Reticle)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWCaptureAim] WBP_CaptureAim is missing Panel_Reticle."));
	}
	if (!Panel_TargetInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWCaptureAim] WBP_CaptureAim is missing Panel_TargetInfo."));
	}

	TargetOnlyWidgets.Reset();
	if (Panel_TargetInfo)
	{
		TargetOnlyWidgets.Add(Panel_TargetInfo);
	}

	RefreshCaptureAim();
}

void UPWCaptureAimWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!IsDesignTime())
	{
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (Root_CaptureAim)
	{
		Root_CaptureAim->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	if (Panel_Reticle)
	{
		Panel_Reticle->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	if (Panel_TargetInfo)
	{
		Panel_TargetInfo->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	if (Text_TargetName)
	{
		Text_TargetName->SetText(NSLOCTEXT("PWCaptureAim", "DesignTargetName", "꼬꼬닭"));
	}
	if (Text_Chance)
	{
		Text_Chance->SetText(NSLOCTEXT("PWCaptureAim", "DesignChance", "34%"));
	}
	if (Text_SphereCount)
	{
		Text_SphereCount->SetText(NSLOCTEXT("PWCaptureAim", "DesignSphereCount", "남은 스피어 : 6"));
	}
}

void UPWCaptureAimWidget::NativeDestruct()
{
	UnbindCaptureComponent();
	Super::NativeDestruct();
}

void UPWCaptureAimWidget::HandleCaptureAimChanged()
{
	RefreshCaptureAim();
}

void UPWCaptureAimWidget::BindCaptureComponent(UPWPlayerCaptureComponent* InCaptureComponent)
{
	if (BoundCaptureComponent == InCaptureComponent)
	{
		return;
	}

	UnbindCaptureComponent();
	BoundCaptureComponent = InCaptureComponent;
	if (BoundCaptureComponent)
	{
		BoundCaptureComponent->OnCaptureAimInfoChanged.AddUniqueDynamic(this, &UPWCaptureAimWidget::HandleCaptureAimChanged);
	}
}

void UPWCaptureAimWidget::UnbindCaptureComponent()
{
	if (BoundCaptureComponent)
	{
		BoundCaptureComponent->OnCaptureAimInfoChanged.RemoveDynamic(this, &UPWCaptureAimWidget::HandleCaptureAimChanged);
		BoundCaptureComponent = nullptr;
	}
}

void UPWCaptureAimWidget::RefreshCaptureAim()
{
	const bool bVisible = BoundCaptureComponent && BoundCaptureComponent->IsCaptureAimVisible();
	const bool bHasTarget = BoundCaptureComponent && BoundCaptureComponent->HasCaptureAimTarget();
	const ESlateVisibility RootVisibility = bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	const ESlateVisibility TargetVisibility = (bVisible && bHasTarget) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;

	SetVisibility(RootVisibility);
	if (Root_CaptureAim)
	{
		Root_CaptureAim->SetVisibility(RootVisibility);
	}
	if (Panel_Reticle)
	{
		Panel_Reticle->SetVisibility(RootVisibility);
	}
	if (Panel_AimOnly)
	{
		Panel_AimOnly->SetVisibility(RootVisibility);
	}

	SetTargetOnlyWidgetsVisibility(TargetVisibility);

	if (Text_TargetName)
	{
		Text_TargetName->SetText(bHasTarget ? BoundCaptureComponent->GetCaptureAimTargetNameText() : FText::GetEmpty());
	}

	if (Text_Chance)
	{
		Text_Chance->SetText((bVisible && bHasTarget)
			? FText::Format(NSLOCTEXT("PWCaptureAim", "ChancePercent", "{0}%"), FText::AsNumber(BoundCaptureComponent->GetCaptureAimChancePercent()))
			: FText::GetEmpty());
	}

	if (Text_SphereCount)
	{
		Text_SphereCount->SetText(bVisible
			? FText::Format(NSLOCTEXT("PWCaptureAim", "SphereCount", "남은 스피어 : {0}"), FText::AsNumber(BoundCaptureComponent->GetThrowableCaptureSphereCount()))
			: FText::GetEmpty());
	}
}

void UPWCaptureAimWidget::SetTargetOnlyWidgetsVisibility(ESlateVisibility NewVisibility)
{
	for (UWidget* TargetOnlyWidget : TargetOnlyWidgets)
	{
		if (TargetOnlyWidget)
		{
			TargetOnlyWidget->SetVisibility(NewVisibility);
		}
	}
}
