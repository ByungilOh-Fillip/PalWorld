#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Session/PW_SessionSubsystem.h"
#include "PW_SessionSlotWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPW_OnSessionSlotJoinRequested, int32, SearchIndex);

UCLASS(Blueprintable)
class PALWORLD_API UPW_SessionSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "PW|Session|UI")
	void InitializeSessionSlot(const FPW_SessionSearchResult& InSearchResult);

	UFUNCTION(BlueprintPure, Category = "PW|Session|UI")
	FPW_SessionSearchResult GetSearchResult() const { return SearchResult; }

	UPROPERTY(BlueprintAssignable, Category = "PW|Session|UI")
	FPW_OnSessionSlotJoinRequested OnJoinRequested;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Session|UI", meta = (DisplayName = "On Session Slot Updated"))
	void BP_OnSessionSlotUpdated();

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_JoinSession;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RoomName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_HostNickname;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_PlayerCount;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Ping;

	UPROPERTY(BlueprintReadOnly, Category = "PW|Session|UI", meta = (AllowPrivateAccess = "true"))
	FPW_SessionSearchResult SearchResult;

	UFUNCTION()
	void HandleJoinClicked();

	void RefreshTexts();
};
