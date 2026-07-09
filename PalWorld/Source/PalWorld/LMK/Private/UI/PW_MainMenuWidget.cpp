#include "UI/PW_MainMenuWidget.h"

#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SafeZone.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"
#include "Components/SpinBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Session/PW_SessionSubsystem.h"
#include "UI/PW_OptionsWidget.h"
#include "UI/PW_SessionSlotWidget.h"
#include "Widgets/SWidget.h"

namespace
{
	constexpr int32 MainTitlePageIndex = 0;
	constexpr int32 PlayModeSelectPageIndex = 1;
	constexpr int32 CreateRoomPageIndex = 2;
	constexpr int32 FindRoomPageIndex = 3;
	constexpr int32 DedicatedUnavailablePageIndex = 4;
	constexpr int32 OptionsPageIndex = 5;
}

TSharedRef<SWidget> UPW_MainMenuWidget::RebuildWidget()
{
	if (MenuSwitcher == nullptr)
	{
		BuildFallbackLayout();
	}

	return Super::RebuildWidget();
}

void UPW_MainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindButtonEvents();
	BindSessionEvents();

	if (Edit_Nickname && Edit_Nickname->GetText().IsEmpty())
	{
		Edit_Nickname->SetText(FText::FromString(DefaultNickname));
	}

	if (SpinBox_MaxPlayers)
	{
		SpinBox_MaxPlayers->SetMinValue(2.0f);
		SpinBox_MaxPlayers->SetMaxValue(64.0f);
		SpinBox_MaxPlayers->SetValue(DefaultMaxPlayers);
	}

	SetMultiplayerEnabled(false);
	ShowMainTitle();
}

void UPW_MainMenuWidget::NativeDestruct()
{
	UnbindSessionEvents();
	UnbindButtonEvents();
	Super::NativeDestruct();
}

void UPW_MainMenuWidget::ShowMainTitle()
{
	SetPageIndex(MainTitlePageIndex);
	SetStatusMessage(FText::GetEmpty());
}

void UPW_MainMenuWidget::ShowPlayModeSelect()
{
	SetPageIndex(PlayModeSelectPageIndex);
	RefreshPlayModeControls();
}

void UPW_MainMenuWidget::ShowCreateRoom()
{
	RefreshRoomDefaults();
	SetPageIndex(CreateRoomPageIndex);
}

void UPW_MainMenuWidget::ShowFindRoom()
{
	SetPageIndex(FindRoomPageIndex);
	HandleRefreshSessionsClicked();
}

void UPW_MainMenuWidget::ShowDedicatedUnavailable()
{
	SetPageIndex(DedicatedUnavailablePageIndex);
	SetStatusMessage(NSLOCTEXT("PWMainMenu", "DedicatedUnavailable", "Dedicated server support is not implemented yet."));
}

void UPW_MainMenuWidget::ShowOptions()
{
	SetPageIndex(OptionsPageIndex);
}

void UPW_MainMenuWidget::SetMultiplayerEnabled(bool bEnabled)
{
	if (bMultiplayerEnabled == bEnabled)
	{
		RefreshPlayModeControls();
		return;
	}

	bMultiplayerEnabled = bEnabled;
	RefreshPlayModeControls();
	BP_OnMultiplayerModeChanged(bMultiplayerEnabled);
}

void UPW_MainMenuWidget::BindButtonEvents()
{
	if (Button_GameStart)
	{
		Button_GameStart->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleGameStartClicked);
	}

	if (Button_DedicatedServer)
	{
		Button_DedicatedServer->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleDedicatedServerClicked);
	}

	if (Button_Options)
	{
		Button_Options->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleOptionsClicked);
	}

	if (Button_QuitGame)
	{
		Button_QuitGame->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleQuitGameClicked);
	}

	if (Button_MultiplayerOff)
	{
		Button_MultiplayerOff->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleMultiplayerOffClicked);
	}

	if (Button_MultiplayerOn)
	{
		Button_MultiplayerOn->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleMultiplayerOnClicked);
	}

	if (Button_PlayStart)
	{
		Button_PlayStart->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandlePlayStartClicked);
	}

	if (Button_PlayBack)
	{
		Button_PlayBack->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandlePlayBackClicked);
	}

	if (Button_CreateRoom)
	{
		Button_CreateRoom->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleCreateRoomClicked);
	}

	if (Button_FindRoom)
	{
		Button_FindRoom->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleFindRoomClicked);
	}

	if (Button_CreateRoomStart)
	{
		Button_CreateRoomStart->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleCreateRoomStartClicked);
	}

	if (Button_CreateRoomBack)
	{
		Button_CreateRoomBack->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleCreateRoomBackClicked);
	}

	if (Button_RefreshSessions)
	{
		Button_RefreshSessions->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleRefreshSessionsClicked);
	}

	if (Button_FindRoomBack)
	{
		Button_FindRoomBack->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleFindRoomBackClicked);
	}

	if (Button_DedicatedBack)
	{
		Button_DedicatedBack->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleDedicatedBackClicked);
	}

	if (Button_OptionsBack)
	{
		Button_OptionsBack->OnClicked.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleOptionsBackClicked);
	}

	if (Edit_Nickname)
	{
		Edit_Nickname->OnTextChanged.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleNicknameTextChanged);
	}

	if (OptionsWidget)
	{
		OptionsWidget->OnOptionsClosed.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleOptionsClosed);
	}
}

void UPW_MainMenuWidget::UnbindButtonEvents()
{
	if (Button_GameStart)
	{
		Button_GameStart->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleGameStartClicked);
	}

	if (Button_DedicatedServer)
	{
		Button_DedicatedServer->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleDedicatedServerClicked);
	}

	if (Button_Options)
	{
		Button_Options->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleOptionsClicked);
	}

	if (Button_QuitGame)
	{
		Button_QuitGame->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleQuitGameClicked);
	}

	if (Button_MultiplayerOff)
	{
		Button_MultiplayerOff->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleMultiplayerOffClicked);
	}

	if (Button_MultiplayerOn)
	{
		Button_MultiplayerOn->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleMultiplayerOnClicked);
	}

	if (Button_PlayStart)
	{
		Button_PlayStart->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandlePlayStartClicked);
	}

	if (Button_PlayBack)
	{
		Button_PlayBack->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandlePlayBackClicked);
	}

	if (Button_CreateRoom)
	{
		Button_CreateRoom->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleCreateRoomClicked);
	}

	if (Button_FindRoom)
	{
		Button_FindRoom->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleFindRoomClicked);
	}

	if (Button_CreateRoomStart)
	{
		Button_CreateRoomStart->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleCreateRoomStartClicked);
	}

	if (Button_CreateRoomBack)
	{
		Button_CreateRoomBack->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleCreateRoomBackClicked);
	}

	if (Button_RefreshSessions)
	{
		Button_RefreshSessions->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleRefreshSessionsClicked);
	}

	if (Button_FindRoomBack)
	{
		Button_FindRoomBack->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleFindRoomBackClicked);
	}

	if (Button_DedicatedBack)
	{
		Button_DedicatedBack->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleDedicatedBackClicked);
	}

	if (Button_OptionsBack)
	{
		Button_OptionsBack->OnClicked.RemoveDynamic(this, &UPW_MainMenuWidget::HandleOptionsBackClicked);
	}

	if (Edit_Nickname)
	{
		Edit_Nickname->OnTextChanged.RemoveDynamic(this, &UPW_MainMenuWidget::HandleNicknameTextChanged);
	}

	if (OptionsWidget)
	{
		OptionsWidget->OnOptionsClosed.RemoveDynamic(this, &UPW_MainMenuWidget::HandleOptionsClosed);
	}
}

void UPW_MainMenuWidget::BindSessionEvents()
{
	UPW_SessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (SessionSubsystem == nullptr)
	{
		return;
	}

	SessionSubsystem->OnSessionOperationFinished.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleSessionOperationFinished);
	SessionSubsystem->OnSessionSearchResult.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleSessionSearchResult);
	SessionSubsystem->OnSessionSearchStateChanged.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleSessionSearchStateChanged);
	SessionSubsystem->SetGameMapPath(MakeGameMapTravelPath());
}

void UPW_MainMenuWidget::UnbindSessionEvents()
{
	UPW_SessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (SessionSubsystem == nullptr)
	{
		return;
	}

	SessionSubsystem->OnSessionOperationFinished.RemoveDynamic(this, &UPW_MainMenuWidget::HandleSessionOperationFinished);
	SessionSubsystem->OnSessionSearchResult.RemoveDynamic(this, &UPW_MainMenuWidget::HandleSessionSearchResult);
	SessionSubsystem->OnSessionSearchStateChanged.RemoveDynamic(this, &UPW_MainMenuWidget::HandleSessionSearchStateChanged);
}

void UPW_MainMenuWidget::SetPageIndex(int32 PageIndex)
{
	if (MenuSwitcher && MenuSwitcher->GetNumWidgets() > PageIndex)
	{
		MenuSwitcher->SetActiveWidgetIndex(PageIndex);
	}
}

void UPW_MainMenuWidget::RefreshPlayModeControls()
{
	const bool bHasNickname = !GetNickname().IsEmpty();
	if (Button_PlayStart)
	{
		Button_PlayStart->SetVisibility(bMultiplayerEnabled ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	if (Button_CreateRoom)
	{
		Button_CreateRoom->SetVisibility(bMultiplayerEnabled ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Button_CreateRoom->SetIsEnabled(bHasNickname && !bSessionOperationInProgress);
	}

	if (Button_FindRoom)
	{
		Button_FindRoom->SetVisibility(bMultiplayerEnabled ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Button_FindRoom->SetIsEnabled(bHasNickname && !bSessionOperationInProgress);
	}

	if (Edit_Nickname)
	{
		Edit_Nickname->SetVisibility(bMultiplayerEnabled ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UPW_MainMenuWidget::RefreshRoomDefaults()
{
	if (Edit_RoomName && Edit_RoomName->GetText().IsEmpty())
	{
		Edit_RoomName->SetText(FText::FromString(FString::Printf(TEXT("%s's World"), *GetNickname())));
	}
}

void UPW_MainMenuWidget::SetStatusMessage(const FText& Message)
{
	if (Text_StatusMessage)
	{
		Text_StatusMessage->SetText(Message);
	}

	BP_OnStatusMessageChanged(Message);
}

void UPW_MainMenuWidget::SetSessionButtonsEnabled(bool bEnabled)
{
	bSessionOperationInProgress = !bEnabled;

	if (Button_CreateRoomStart)
	{
		Button_CreateRoomStart->SetIsEnabled(bEnabled);
	}

	if (Button_RefreshSessions)
	{
		Button_RefreshSessions->SetIsEnabled(bEnabled);
	}

	RefreshPlayModeControls();
}

FString UPW_MainMenuWidget::MakeGameMapTravelPath() const
{
	FString TravelPath = GameMapName.ToString();
	if (!GameMapTravelOptions.IsEmpty())
	{
		TravelPath += TEXT("?");
		TravelPath += GameMapTravelOptions;
	}

	return TravelPath;
}

FString UPW_MainMenuWidget::GetNickname() const
{
	return Edit_Nickname ? Edit_Nickname->GetText().ToString().TrimStartAndEnd() : DefaultNickname;
}

FString UPW_MainMenuWidget::GetRoomName() const
{
	return Edit_RoomName ? Edit_RoomName->GetText().ToString().TrimStartAndEnd() : FString::Printf(TEXT("%s's World"), *GetNickname());
}

int32 UPW_MainMenuWidget::GetMaxPlayers() const
{
	return SpinBox_MaxPlayers ? FMath::RoundToInt(SpinBox_MaxPlayers->GetValue()) : DefaultMaxPlayers;
}

UPW_SessionSubsystem* UPW_MainMenuWidget::GetSessionSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UPW_SessionSubsystem>() : nullptr;
}

void UPW_MainMenuWidget::BuildFallbackLayout()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	if (BackgroundTexture.IsNull())
	{
		BackgroundTexture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/_Private/LMK/UI/Menu/T_MainMenu_Background.T_MainMenu_Background")));
	}

	if (SessionSlotWidgetClass == nullptr)
	{
		SessionSlotWidgetClass = UPW_SessionSlotWidget::StaticClass();
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	AddFullscreenBackground(RootCanvas);
	AddTitleLayer(RootCanvas);

	MenuSwitcher = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("MenuSwitcher"));
	MenuSwitcher->AddChild(BuildMainTitlePage());
	MenuSwitcher->AddChild(BuildPlayModeSelectPage());
	MenuSwitcher->AddChild(BuildCreateRoomPage());
	MenuSwitcher->AddChild(BuildFindRoomPage());
	MenuSwitcher->AddChild(BuildDedicatedUnavailablePage());
	MenuSwitcher->AddChild(BuildOptionsPage());

	UCanvasPanelSlot* SwitcherSlot = RootCanvas->AddChildToCanvas(MenuSwitcher);
	SwitcherSlot->SetAnchors(FAnchors(0.5f, 0.66f));
	SwitcherSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	SwitcherSlot->SetSize(FVector2D(760.0f, 430.0f));

	AddCornerTexts(RootCanvas);
}

void UPW_MainMenuWidget::AddFullscreenBackground(UCanvasPanel* RootCanvas)
{
	UScaleBox* BackgroundScaleBox = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("BackgroundScaleBox"));
	BackgroundScaleBox->SetStretch(EStretch::ScaleToFill);
	BackgroundScaleBox->SetStretchDirection(EStretchDirection::Both);

	UImage* BackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Image_Background"));
	if (UTexture2D* LoadedBackground = BackgroundTexture.LoadSynchronous())
	{
		BackgroundImage->SetBrushFromTexture(LoadedBackground, true);
	}
	else
	{
		BackgroundImage->SetColorAndOpacity(FLinearColor(0.08f, 0.18f, 0.22f, 1.0f));
	}
	BackgroundScaleBox->AddChild(BackgroundImage);

	UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(BackgroundScaleBox);
	BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	BackgroundSlot->SetOffsets(FMargin(0.0f));

	UImage* Vignette = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Image_MenuVignette"));
	Vignette->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.18f));
	UCanvasPanelSlot* VignetteSlot = RootCanvas->AddChildToCanvas(Vignette);
	VignetteSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	VignetteSlot->SetOffsets(FMargin(0.0f));
}

void UPW_MainMenuWidget::AddTitleLayer(UCanvasPanel* RootCanvas)
{
	UVerticalBox* TitleBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TitleBox"));

	UTextBlock* TitleText = MakeMenuText(FText::FromString(TEXT("PALWORLD")), 128, 1.0f);
	TitleText->SetJustification(ETextJustify::Center);
	FSlateFontInfo TitleFont = TitleText->GetFont();
	TitleFont.OutlineSettings.OutlineSize = 1;
	TitleText->SetFont(TitleFont);
	TitleBox->AddChildToVerticalBox(TitleText);

	UCanvasPanelSlot* TitleSlot = RootCanvas->AddChildToCanvas(TitleBox);
	TitleSlot->SetAnchors(FAnchors(0.5f, 0.36f));
	TitleSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	TitleSlot->SetSize(FVector2D(980.0f, 180.0f));
}

void UPW_MainMenuWidget::AddCornerTexts(UCanvasPanel* RootCanvas)
{
	UTextBlock* BottomLeftText = MakeMenuText(FText::FromString(TEXT("Palworld는 현재 얼리 액세스 단계입니다.\n오작동이나 충돌이 발생할 수 있습니다.")), 20, 0.95f);
	UCanvasPanelSlot* BottomLeftSlot = RootCanvas->AddChildToCanvas(BottomLeftText);
	BottomLeftSlot->SetAnchors(FAnchors(0.0f, 1.0f));
	BottomLeftSlot->SetAlignment(FVector2D(0.0f, 1.0f));
	BottomLeftSlot->SetOffsets(FMargin(34.0f, -72.0f, 620.0f, 64.0f));
}

UWidget* UPW_MainMenuWidget::BuildMainTitlePage()
{
	UVerticalBox* PageBox = MakePageBox();
	Button_GameStart = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackGameStart", "게임 시작"));
	Button_DedicatedServer = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackDedicated", "멀티 플레이 참가하기 (전용 서버)"));
	PageBox->AddChildToVerticalBox(Button_GameStart);
	PageBox->AddChildToVerticalBox(Button_DedicatedServer);
	PageBox->AddChildToVerticalBox(MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackInvite", "멀티 플레이 참가하기 (초대 코드)")));
	PageBox->AddChildToVerticalBox(MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackPalBox", "글로벌 팰 상자")));
	PageBox->AddChildToVerticalBox(MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackGuide", "생존 가이드")));
	Button_Options = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackOptions", "옵션"));
	PageBox->AddChildToVerticalBox(Button_Options);
	PageBox->AddChildToVerticalBox(MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackNews", "뉴스")));
	PageBox->AddChildToVerticalBox(MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackCredits", "제작진")));
	Button_QuitGame = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackQuit", "게임 종료"));
	PageBox->AddChildToVerticalBox(Button_QuitGame);
	return PageBox;
}

UWidget* UPW_MainMenuWidget::BuildPlayModeSelectPage()
{
	UVerticalBox* PageBox = MakePageBox();
	PageBox->AddChildToVerticalBox(MakeMenuText(NSLOCTEXT("PWMainMenu", "FallbackPlayMode", "플레이 방식"), 30));
	Button_MultiplayerOff = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackSolo", "싱글 플레이"));
	Button_MultiplayerOn = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackMulti", "멀티 플레이"));
	Button_PlayStart = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackStartWorld", "월드 시작"));
	Button_CreateRoom = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackCreateRoom", "방 만들기"));
	Button_FindRoom = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackFindRoom", "방 찾기"));
	Button_PlayBack = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackBack", "뒤로"));
	Edit_Nickname = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("Edit_Nickname"));
	Edit_Nickname->SetHintText(NSLOCTEXT("PWMainMenu", "FallbackNicknameHint", "닉네임"));
	PageBox->AddChildToVerticalBox(Button_MultiplayerOff);
	PageBox->AddChildToVerticalBox(Button_MultiplayerOn);
	PageBox->AddChildToVerticalBox(Edit_Nickname);
	PageBox->AddChildToVerticalBox(Button_PlayStart);
	PageBox->AddChildToVerticalBox(Button_CreateRoom);
	PageBox->AddChildToVerticalBox(Button_FindRoom);
	PageBox->AddChildToVerticalBox(Button_PlayBack);
	return PageBox;
}

UWidget* UPW_MainMenuWidget::BuildCreateRoomPage()
{
	UVerticalBox* PageBox = MakePageBox();
	PageBox->AddChildToVerticalBox(MakeMenuText(NSLOCTEXT("PWMainMenu", "FallbackCreateRoomTitle", "방 만들기"), 30));
	Edit_RoomName = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("Edit_RoomName"));
	Edit_RoomName->SetHintText(NSLOCTEXT("PWMainMenu", "FallbackRoomNameHint", "방 이름"));
	SpinBox_MaxPlayers = WidgetTree->ConstructWidget<USpinBox>(USpinBox::StaticClass(), TEXT("SpinBox_MaxPlayers"));
	Button_CreateRoomStart = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackCreateStart", "시작"));
	Button_CreateRoomBack = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackCreateBack", "뒤로"));
	PageBox->AddChildToVerticalBox(Edit_RoomName);
	PageBox->AddChildToVerticalBox(SpinBox_MaxPlayers);
	PageBox->AddChildToVerticalBox(Button_CreateRoomStart);
	PageBox->AddChildToVerticalBox(Button_CreateRoomBack);
	return PageBox;
}

UWidget* UPW_MainMenuWidget::BuildFindRoomPage()
{
	UVerticalBox* PageBox = MakePageBox();
	PageBox->AddChildToVerticalBox(MakeMenuText(NSLOCTEXT("PWMainMenu", "FallbackFindRoomTitle", "방 찾기"), 30));
	Button_RefreshSessions = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackRefresh", "새로고침"));
	Scroll_RoomList = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("Scroll_RoomList"));
	Button_FindRoomBack = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackFindBack", "뒤로"));
	PageBox->AddChildToVerticalBox(Button_RefreshSessions);
	UVerticalBoxSlot* ListSlot = PageBox->AddChildToVerticalBox(Scroll_RoomList);
	ListSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	PageBox->AddChildToVerticalBox(Button_FindRoomBack);
	return PageBox;
}

UWidget* UPW_MainMenuWidget::BuildDedicatedUnavailablePage()
{
	UVerticalBox* PageBox = MakePageBox();
	PageBox->AddChildToVerticalBox(MakeMenuText(NSLOCTEXT("PWMainMenu", "FallbackDedicatedTitle", "전용 서버"), 30));
	PageBox->AddChildToVerticalBox(MakeMenuText(NSLOCTEXT("PWMainMenu", "FallbackDedicatedBody", "전용 서버 참가 기능은 아직 준비 중입니다."), 22, 0.9f));
	Button_DedicatedBack = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackDedicatedBack", "뒤로"));
	PageBox->AddChildToVerticalBox(Button_DedicatedBack);
	return PageBox;
}

UWidget* UPW_MainMenuWidget::BuildOptionsPage()
{
	UVerticalBox* PageBox = MakePageBox();
	PageBox->AddChildToVerticalBox(MakeMenuText(NSLOCTEXT("PWMainMenu", "FallbackOptionsTitle", "옵션"), 30));
	PageBox->AddChildToVerticalBox(MakeMenuText(NSLOCTEXT("PWMainMenu", "FallbackOptionsBody", "그래픽, 사운드, 조작 설정 화면은 추후 확장됩니다."), 22, 0.9f));
	Text_StatusMessage = MakeMenuText(FText::GetEmpty(), 18, 0.9f);
	Button_OptionsBack = MakeMenuButton(NSLOCTEXT("PWMainMenu", "FallbackOptionsBack", "뒤로"));
	PageBox->AddChildToVerticalBox(Text_StatusMessage);
	PageBox->AddChildToVerticalBox(Button_OptionsBack);
	return PageBox;
}

UButton* UPW_MainMenuWidget::MakeMenuButton(const FText& Label, int32 FontSize) const
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	FButtonStyle ButtonStyle = Button->GetStyle();
	ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
	ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(0.08f, 0.22f, 0.25f, 0.42f));
	ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(0.02f, 0.10f, 0.12f, 0.55f));
	Button->SetStyle(ButtonStyle);

	UTextBlock* Text = MakeMenuText(Label, FontSize);
	Text->SetJustification(ETextJustify::Center);
	Button->AddChild(Text);
	return Button;
}

UTextBlock* UPW_MainMenuWidget::MakeMenuText(const FText& Label, int32 FontSize, float Opacity) const
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Text->SetText(Label);
	Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.88f, 0.98f, 1.0f, Opacity)));
	Text->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
	Text->SetShadowOffset(FVector2D(1.5f, 1.5f));
	FSlateFontInfo Font = Text->GetFont();
	Font.Size = FontSize;
	Text->SetFont(Font);
	return Text;
}

UVerticalBox* UPW_MainMenuWidget::MakePageBox() const
{
	UVerticalBox* PageBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	PageBox->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	return PageBox;
}

void UPW_MainMenuWidget::HandleGameStartClicked()
{
	ShowPlayModeSelect();
}

void UPW_MainMenuWidget::HandleDedicatedServerClicked()
{
	ShowDedicatedUnavailable();
}

void UPW_MainMenuWidget::HandleOptionsClicked()
{
	ShowOptions();
}

void UPW_MainMenuWidget::HandleQuitGameClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UPW_MainMenuWidget::HandleMultiplayerOffClicked()
{
	SetMultiplayerEnabled(false);
}

void UPW_MainMenuWidget::HandleMultiplayerOnClicked()
{
	SetMultiplayerEnabled(true);
}

void UPW_MainMenuWidget::HandlePlayStartClicked()
{
	UGameplayStatics::OpenLevel(this, GameMapName, true, GameMapTravelOptions);
}

void UPW_MainMenuWidget::HandlePlayBackClicked()
{
	ShowMainTitle();
}

void UPW_MainMenuWidget::HandleCreateRoomClicked()
{
	ShowCreateRoom();
}

void UPW_MainMenuWidget::HandleFindRoomClicked()
{
	ShowFindRoom();
}

void UPW_MainMenuWidget::HandleCreateRoomStartClicked()
{
	if (GetNickname().IsEmpty())
	{
		SetStatusMessage(NSLOCTEXT("PWMainMenu", "NicknameRequired", "Enter a nickname first."));
		return;
	}

	UPW_SessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (SessionSubsystem == nullptr)
	{
		SetStatusMessage(NSLOCTEXT("PWMainMenu", "SessionSubsystemMissing", "Session subsystem is not available."));
		return;
	}

	SetSessionButtonsEnabled(false);
	SetStatusMessage(NSLOCTEXT("PWMainMenu", "CreatingSession", "Creating room..."));
	SessionSubsystem->SetGameMapPath(MakeGameMapTravelPath());
	SessionSubsystem->CreateListenSession(GetRoomName(), GetNickname(), GetMaxPlayers());
}

void UPW_MainMenuWidget::HandleCreateRoomBackClicked()
{
	ShowPlayModeSelect();
}

void UPW_MainMenuWidget::HandleRefreshSessionsClicked()
{
	if (Scroll_RoomList)
	{
		Scroll_RoomList->ClearChildren();
	}

	UPW_SessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (SessionSubsystem == nullptr)
	{
		SetStatusMessage(NSLOCTEXT("PWMainMenu", "FindSessionSubsystemMissing", "Session subsystem is not available."));
		return;
	}

	SetStatusMessage(NSLOCTEXT("PWMainMenu", "FindingSessions", "Finding rooms..."));
	SessionSubsystem->FindSessions();
}

void UPW_MainMenuWidget::HandleFindRoomBackClicked()
{
	ShowPlayModeSelect();
}

void UPW_MainMenuWidget::HandleDedicatedBackClicked()
{
	ShowMainTitle();
}

void UPW_MainMenuWidget::HandleOptionsBackClicked()
{
	ShowMainTitle();
}

void UPW_MainMenuWidget::HandleNicknameTextChanged(const FText& Text)
{
	RefreshPlayModeControls();
}

void UPW_MainMenuWidget::HandleSessionOperationFinished(EPW_SessionOperation Operation, bool bWasSuccessful, FText Message)
{
	if (Operation == EPW_SessionOperation::Create || Operation == EPW_SessionOperation::Join)
	{
		SetSessionButtonsEnabled(true);
	}

	SetStatusMessage(Message);
}

void UPW_MainMenuWidget::HandleSessionSearchResult(const FPW_SessionSearchResult& SearchResult)
{
	if (Scroll_RoomList == nullptr || SessionSlotWidgetClass == nullptr)
	{
		return;
	}

	UPW_SessionSlotWidget* SlotWidget = CreateWidget<UPW_SessionSlotWidget>(GetOwningPlayer(), SessionSlotWidgetClass);
	if (SlotWidget == nullptr)
	{
		return;
	}

	SlotWidget->InitializeSessionSlot(SearchResult);
	SlotWidget->OnJoinRequested.AddUniqueDynamic(this, &UPW_MainMenuWidget::HandleSessionSlotJoinRequested);
	Scroll_RoomList->AddChild(SlotWidget);
}

void UPW_MainMenuWidget::HandleSessionSearchStateChanged(bool bSearching)
{
	if (Button_RefreshSessions)
	{
		Button_RefreshSessions->SetIsEnabled(!bSearching);
	}
}

void UPW_MainMenuWidget::HandleSessionSlotJoinRequested(int32 SearchIndex)
{
	if (GetNickname().IsEmpty())
	{
		SetStatusMessage(NSLOCTEXT("PWMainMenu", "JoinNicknameRequired", "Enter a nickname first."));
		return;
	}

	UPW_SessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (SessionSubsystem == nullptr)
	{
		SetStatusMessage(NSLOCTEXT("PWMainMenu", "JoinSessionSubsystemMissing", "Session subsystem is not available."));
		return;
	}

	SetSessionButtonsEnabled(false);
	SetStatusMessage(NSLOCTEXT("PWMainMenu", "JoiningSession", "Joining room..."));
	SessionSubsystem->JoinSession(SearchIndex, GetNickname());
}

void UPW_MainMenuWidget::HandleOptionsClosed()
{
	ShowMainTitle();
}
