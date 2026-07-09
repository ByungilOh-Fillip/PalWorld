#include "PWInteractionGuideWidget.h"

#include "Components/VerticalBox.h"
#include "PWInteractionGuideEntryWidget.h"

void UPWInteractionGuideWidget::SetGuideActions(const TArray<FPWInteractionGuideAction>& NewActions)
{
	GuideActions = NewActions;
	RebuildGuideList();
}

void UPWInteractionGuideWidget::NativeDestruct()
{
	EntryWidgets.Reset();
	Super::NativeDestruct();
}

void UPWInteractionGuideWidget::RebuildGuideList()
{
	if (GuideList == nullptr)
	{
		return;
	}

	GuideList->ClearChildren();
	EntryWidgets.Reset();

	TSubclassOf<UPWInteractionGuideEntryWidget> ActiveEntryWidgetClass = EntryWidgetClass;
	if (ActiveEntryWidgetClass == nullptr)
	{
		ActiveEntryWidgetClass = UPWInteractionGuideEntryWidget::StaticClass();
	}

	for (const FPWInteractionGuideAction& GuideAction : GuideActions)
	{
		UPWInteractionGuideEntryWidget* EntryWidget = CreateWidget<UPWInteractionGuideEntryWidget>(GetWorld(), ActiveEntryWidgetClass);
		if (EntryWidget == nullptr)
		{
			continue;
		}

		EntryWidget->SetGuideAction(GuideAction);
		GuideList->AddChild(EntryWidget);
		EntryWidgets.Add(EntryWidget);
	}
}
