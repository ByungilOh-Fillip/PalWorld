#include "UI/PW_SessionSlotWidget.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"

void UPW_SessionSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_JoinSession == nullptr && WidgetTree)
	{
		UHorizontalBox* RootBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RootBox"));
		WidgetTree->RootWidget = RootBox;

		Text_RoomName = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_RoomName"));
		Text_HostNickname = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_HostNickname"));
		Text_PlayerCount = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_PlayerCount"));
		Text_Ping = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Ping"));
		Button_JoinSession = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_JoinSession"));

		UTextBlock* JoinText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Join"));
		JoinText->SetText(NSLOCTEXT("PWSessionSlot", "Join", "참가"));
		Button_JoinSession->AddChild(JoinText);

		RootBox->AddChildToHorizontalBox(Text_RoomName)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RootBox->AddChildToHorizontalBox(Text_HostNickname)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RootBox->AddChildToHorizontalBox(Text_PlayerCount);
		RootBox->AddChildToHorizontalBox(Text_Ping);
		RootBox->AddChildToHorizontalBox(Button_JoinSession);
	}

	if (Button_JoinSession)
	{
		Button_JoinSession->OnClicked.AddUniqueDynamic(this, &UPW_SessionSlotWidget::HandleJoinClicked);
	}

	RefreshTexts();
}

void UPW_SessionSlotWidget::NativeDestruct()
{
	if (Button_JoinSession)
	{
		Button_JoinSession->OnClicked.RemoveDynamic(this, &UPW_SessionSlotWidget::HandleJoinClicked);
	}

	Super::NativeDestruct();
}

void UPW_SessionSlotWidget::InitializeSessionSlot(const FPW_SessionSearchResult& InSearchResult)
{
	SearchResult = InSearchResult;
	RefreshTexts();
	BP_OnSessionSlotUpdated();
}

void UPW_SessionSlotWidget::HandleJoinClicked()
{
	if (SearchResult.SearchIndex != INDEX_NONE)
	{
		OnJoinRequested.Broadcast(SearchResult.SearchIndex);
	}
}

void UPW_SessionSlotWidget::RefreshTexts()
{
	if (Text_RoomName)
	{
		Text_RoomName->SetText(FText::FromString(SearchResult.RoomName));
	}

	if (Text_HostNickname)
	{
		Text_HostNickname->SetText(FText::FromString(SearchResult.HostNickname));
	}

	if (Text_PlayerCount)
	{
		Text_PlayerCount->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), SearchResult.CurrentPlayers, SearchResult.MaxPlayers)));
	}

	if (Text_Ping)
	{
		Text_Ping->SetText(FText::FromString(FString::Printf(TEXT("%dms"), SearchResult.PingMs)));
	}
}
