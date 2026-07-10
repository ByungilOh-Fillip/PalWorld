// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Components/PWPlayerPalStorageComponent.h"
#include "PWPalPartyPanelWidget.generated.h"

class APWPlayerCharacter;
class UPanelWidget;
class UTextBlock;
class UPWPalPartySlotWidget;
class UPWPlayerPalStorageComponent;

UCLASS(Blueprintable)
class PALWORLD_API UPWPalPartyPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|UI|Pal")
	void InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter);

	UFUNCTION(BlueprintCallable, Category = "Player|UI|Pal")
	void InitializeWithPalStorageComponent(UPWPlayerPalStorageComponent* InPalStorageComponent);

	UFUNCTION(BlueprintCallable, Category = "Player|UI|Pal")
	void RefreshPalParty();

	UFUNCTION(BlueprintPure, Category = "Player|UI|Pal")
	UPWPlayerPalStorageComponent* GetPalStorageComponent() const { return BoundPalStorageComponent; }

	UFUNCTION(BlueprintPure, Category = "Player|UI|Pal")
	TArray<FPWCapturedPalRecord> GetCapturedPals() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|UI|Pal", meta = (DisplayName = "On Pal Party Changed"))
	void BP_OnPalPartyChanged();

	// 슬롯을 직접 배치하지 않을 경우, 여기에 WBP_PalPartySlot을 자동 생성한다.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UPanelWidget> Panel_PartySlots = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UTextBlock> Text_Empty = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UTextBlock> Text_Count = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UPWPalPartySlotWidget> Slot_Party0 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UPWPalPartySlotWidget> Slot_Party1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UPWPalPartySlotWidget> Slot_Party2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UPWPalPartySlotWidget> Slot_Party3 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Pal|Bind")
	TObjectPtr<UPWPalPartySlotWidget> Slot_Party4 = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|UI|Pal|Classes")
	TSubclassOf<UPWPalPartySlotWidget> PartySlotWidgetClass;

private:
	UFUNCTION()
	void HandleCapturedPalsChanged();

	void UnbindPalStorageComponent();
	void RefreshFixedSlots(const TArray<FPWCapturedPalRecord>& CapturedPals);
	void RebuildGeneratedSlots(const TArray<FPWCapturedPalRecord>& CapturedPals);
	bool HasFixedPartySlots() const;

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerPalStorageComponent> BoundPalStorageComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPWPalPartySlotWidget>> GeneratedSlotWidgets;
};
