#include "UI/PW_OptionsWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

void UPW_OptionsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_CloseOptions == nullptr && WidgetTree)
	{
		UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RootBox"));
		WidgetTree->RootWidget = RootBox;

		UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_OptionsTitle"));
		TitleText->SetText(NSLOCTEXT("PWOptions", "Title", "옵션"));
		RootBox->AddChildToVerticalBox(TitleText);

		Button_CloseOptions = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_CloseOptions"));
		UTextBlock* CloseText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_CloseOptions"));
		CloseText->SetText(NSLOCTEXT("PWOptions", "Close", "뒤로"));
		Button_CloseOptions->AddChild(CloseText);
		RootBox->AddChildToVerticalBox(Button_CloseOptions);
	}

	if (Button_CloseOptions)
	{
		Button_CloseOptions->OnClicked.AddUniqueDynamic(this, &UPW_OptionsWidget::HandleCloseClicked);
	}
}

void UPW_OptionsWidget::NativeDestruct()
{
	if (Button_CloseOptions)
	{
		Button_CloseOptions->OnClicked.RemoveDynamic(this, &UPW_OptionsWidget::HandleCloseClicked);
	}

	Super::NativeDestruct();
}

void UPW_OptionsWidget::HandleCloseClicked()
{
	OnOptionsClosed.Broadcast();
}
