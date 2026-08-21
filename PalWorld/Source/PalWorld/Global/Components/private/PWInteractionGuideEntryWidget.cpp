#include "PWInteractionGuideEntryWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPWInteractionGuideEntryWidget::SetGuideAction(const FPWInteractionGuideAction& NewAction)
{
	GuideAction = NewAction;

	if (KeyText)
	{
		KeyText->SetText(FText::FromName(GuideAction.Key.GetFName()));
	}

	if (LabelText)
	{
		LabelText->SetText(GuideAction.Label);
	}

	UProgressBar* BoundProgressBar = ProgressBar ? ProgressBar.Get() : HoldProgressBar.Get();
	if (BoundProgressBar)
	{
		BoundProgressBar->SetPercent(FMath::Clamp(GuideAction.Progress, 0.0f, 1.0f));
		BoundProgressBar->SetVisibility(GuideAction.bShowProgress ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	SetIsEnabled(GuideAction.bEnabled);
}
