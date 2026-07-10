// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Components/PWPlayerPalStorageComponent.h"
#include "PWPalPartySlotWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UWidget;

UCLASS(Blueprintable)
class PALWORLD_API UPWPalPartySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|UI|Pal")
	void SetPalRecord(const FPWCapturedPalRecord& InPalRecord, bool bInHasPal);

	UFUNCTION(BlueprintPure, Category = "Player|UI|Pal")
	bool HasPal() const { return bHasPal; }

	UFUNCTION(BlueprintPure, Category = "Player|UI|Pal")
	FPWCapturedPalRecord GetPalRecord() const { return PalRecord; }

protected:
	virtual void NativePreConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Pal", meta = (DisplayName = "On Pal Slot Updated"))
	void BP_OnPalSlotUpdated();

	UPROPERTY(BlueprintReadOnly, Category = "Player|UI|Pal")
	FPWCapturedPalRecord PalRecord;

	UPROPERTY(BlueprintReadOnly, Category = "Player|UI|Pal")
	bool bHasPal = false;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UWidget> Panel_PalRoot = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UTextBlock> Text_Name = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UTextBlock> Text_Level = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UTextBlock> Text_HP = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UProgressBar> Progress_HP = nullptr;

private:
	void RefreshBoundWidgets();
};
