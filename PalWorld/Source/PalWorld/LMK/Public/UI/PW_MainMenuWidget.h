#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Session/PW_SessionSubsystem.h"
#include "PW_MainMenuWidget.generated.h"

class UButton;
class UCanvasPanel;
class UEditableTextBox;
class UScrollBox;
class USpinBox;
class UTexture2D;
class UTextBlock;
class UVerticalBox;
class UPW_OptionsWidget;
class UPW_SessionSlotWidget;
class UWidget;
class UWidgetSwitcher;
class SWidget;

UCLASS(Blueprintable)
class PALWORLD_API UPW_MainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "PW|Main Menu")
	void ShowMainTitle();

	UFUNCTION(BlueprintCallable, Category = "PW|Main Menu")
	void ShowPlayModeSelect();

	UFUNCTION(BlueprintCallable, Category = "PW|Main Menu")
	void ShowCreateRoom();

	UFUNCTION(BlueprintCallable, Category = "PW|Main Menu")
	void ShowFindRoom();

	UFUNCTION(BlueprintCallable, Category = "PW|Main Menu")
	void ShowDedicatedUnavailable();

	UFUNCTION(BlueprintCallable, Category = "PW|Main Menu")
	void ShowOptions();

	UFUNCTION(BlueprintCallable, Category = "PW|Main Menu")
	void SetMultiplayerEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "PW|Main Menu")
	bool IsMultiplayerEnabled() const { return bMultiplayerEnabled; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Main Menu", meta = (DisplayName = "On Multiplayer Mode Changed"))
	void BP_OnMultiplayerModeChanged(bool bEnabled);

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Main Menu", meta = (DisplayName = "On Status Message Changed"))
	void BP_OnStatusMessageChanged(const FText& Message);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Main Menu", meta = (AllowPrivateAccess = "true"))
	FName GameMapName = FName(TEXT("/Game/_Private/LMK/Levels/Palworld"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Main Menu", meta = (AllowPrivateAccess = "true"))
	FString GameMapTravelOptions = TEXT("game=/Game/_Private/LMK/BluePrints/BP_PWGameMode.BP_PWGameMode_C");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Main Menu", meta = (AllowPrivateAccess = "true", ClampMin = "2", ClampMax = "20"))
	int32 DefaultMaxPlayers = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Main Menu", meta = (AllowPrivateAccess = "true"))
	FString DefaultNickname = TEXT("Player");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Main Menu", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UTexture2D> BackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Main Menu", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPW_SessionSlotWidget> SessionSlotWidgetClass;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> MenuSwitcher;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_GameStart;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_DedicatedServer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Options;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_QuitGame;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_MultiplayerOff;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_MultiplayerOn;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_PlayBack;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_CreateRoom;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_FindRoom;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_CreateRoomStart;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_CreateRoomBack;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_RefreshSessions;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_FindRoomBack;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_DedicatedBack;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_OptionsBack;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> Edit_Nickname;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> Edit_RoomName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USpinBox> SpinBox_MaxPlayers;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> Scroll_RoomList;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_StatusMessage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPW_OptionsWidget> OptionsWidget;

	bool bMultiplayerEnabled = false;
	bool bSessionOperationInProgress = false;

	void BindButtonEvents();
	void UnbindButtonEvents();
	void BindSessionEvents();
	void UnbindSessionEvents();
	void SetPageIndex(int32 PageIndex);
	void RefreshPlayModeControls();
	void ShowMultiplayerMenu();
	void RefreshRoomDefaults();
	void SetStatusMessage(const FText& Message);
	void SetSessionButtonsEnabled(bool bEnabled);
	FString MakeGameMapTravelPath() const;
	FString GetNickname() const;
	FString GetRoomName() const;
	int32 GetMaxPlayers() const;
	UPW_SessionSubsystem* GetSessionSubsystem() const;
	void BuildFallbackLayout();
	void AddFullscreenBackground(UCanvasPanel* RootCanvas);
	void AddTitleLayer(UCanvasPanel* RootCanvas);
	void AddCornerTexts(UCanvasPanel* RootCanvas);
	UWidget* BuildMainTitlePage();
	UWidget* BuildPlayModeSelectPage();
	UWidget* BuildCreateRoomPage();
	UWidget* BuildFindRoomPage();
	UWidget* BuildDedicatedUnavailablePage();
	UWidget* BuildOptionsPage();
	UButton* MakeMenuButton(const FText& Label, int32 FontSize = 24) const;
	UTextBlock* MakeMenuText(const FText& Label, int32 FontSize = 22, float Opacity = 1.0f) const;
	UVerticalBox* MakePageBox() const;

	UFUNCTION()
	void HandleGameStartClicked();

	UFUNCTION()
	void HandleDedicatedServerClicked();

	UFUNCTION()
	void HandleOptionsClicked();

	UFUNCTION()
	void HandleQuitGameClicked();

	UFUNCTION()
	void HandleMultiplayerOffClicked();

	UFUNCTION()
	void HandleMultiplayerOnClicked();

	UFUNCTION()
	void HandlePlayBackClicked();

	UFUNCTION()
	void HandleCreateRoomClicked();

	UFUNCTION()
	void HandleFindRoomClicked();

	UFUNCTION()
	void HandleCreateRoomStartClicked();

	UFUNCTION()
	void HandleCreateRoomBackClicked();

	UFUNCTION()
	void HandleRefreshSessionsClicked();

	UFUNCTION()
	void HandleFindRoomBackClicked();

	UFUNCTION()
	void HandleDedicatedBackClicked();

	UFUNCTION()
	void HandleOptionsBackClicked();

	UFUNCTION()
	void HandleNicknameTextChanged(const FText& Text);

	UFUNCTION()
	void HandleSessionOperationFinished(EPW_SessionOperation Operation, bool bWasSuccessful, FText Message);

	UFUNCTION()
	void HandleSessionSearchResult(const FPW_SessionSearchResult& SearchResult);

	UFUNCTION()
	void HandleSessionSearchStateChanged(bool bSearching);

	UFUNCTION()
	void HandleSessionSlotJoinRequested(int32 SearchIndex);

	UFUNCTION()
	void HandleOptionsClosed();
};
