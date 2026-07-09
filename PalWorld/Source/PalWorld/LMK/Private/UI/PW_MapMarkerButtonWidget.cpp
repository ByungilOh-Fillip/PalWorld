#include "UI/PW_MapMarkerButtonWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "UI/PW_WorldMapWidget.h"

UPW_MapMarkerButtonWidget::UPW_MapMarkerButtonWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InitIsFocusable(false);
}

void UPW_MapMarkerButtonWidget::InitializeMarker(UPW_WorldMapWidget* InOwnerMapWidget, const FPW_MapMarker& InMarker)
{
	OwnerMapWidget = InOwnerMapWidget;
	MarkerType = InMarker.MarkerType;
	MarkerId = InMarker.MarkerId;
	bCanTeleport = InMarker.bCanTeleport;

	SetToolTipText(InMarker.DisplayName);
	SetIsEnabled(true);

	FButtonStyle TransparentButtonStyle = GetStyle();
	TransparentButtonStyle.SetNormal(FSlateNoResource());
	TransparentButtonStyle.SetHovered(FSlateNoResource());
	TransparentButtonStyle.SetPressed(FSlateNoResource());
	TransparentButtonStyle.SetDisabled(FSlateNoResource());
	TransparentButtonStyle.SetNormalPadding(FMargin(0.0f));
	TransparentButtonStyle.SetPressedPadding(FMargin(0.0f));
	SetStyle(TransparentButtonStyle);

	const FLinearColor MarkerTint = OwnerMapWidget != nullptr
		? OwnerMapWidget->GetMarkerTintColor(InMarker)
		: FLinearColor::White;
	SetColorAndOpacity(FLinearColor::White);
	SetBackgroundColor(FLinearColor::Transparent);

	UTexture2D* MarkerIcon = OwnerMapWidget != nullptr
		? OwnerMapWidget->GetMarkerIcon(MarkerType)
		: nullptr;

	if (MarkerIcon != nullptr)
	{
		UImage* IconWidget = NewObject<UImage>(this);
		if (IconWidget != nullptr)
		{
			IconWidget->SetBrushFromTexture(MarkerIcon, true);
			IconWidget->SetColorAndOpacity(MarkerTint);
			SetContent(IconWidget);
		}
	}
	else
	{
		UBorder* ColorWidget = NewObject<UBorder>(this);
		if (ColorWidget != nullptr)
		{
			ColorWidget->SetBrushColor(MarkerTint);
			SetContent(ColorWidget);
		}
	}

	OnClicked.AddUniqueDynamic(this, &UPW_MapMarkerButtonWidget::HandleClicked);
}

void UPW_MapMarkerButtonWidget::HandleClicked()
{
	if (OwnerMapWidget != nullptr && bCanTeleport)
	{
		OwnerMapWidget->SelectMapMarkerForTeleport(MarkerType, MarkerId);
	}
}
