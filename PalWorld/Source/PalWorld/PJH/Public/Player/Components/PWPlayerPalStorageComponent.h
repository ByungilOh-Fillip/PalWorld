// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWPlayerPalStorageComponent.generated.h"

class APWPalBase;

USTRUCT(BlueprintType)
struct PALWORLD_API FPWCapturedPalRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Player|Pal")
	FGuid InstanceId;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Pal")
	FName PalId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Pal")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Pal")
	TSubclassOf<APWPalBase> PalClass;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Pal")
	int32 Level = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Pal")
	float CurrentHP = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Pal")
	float MaxHP = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Pal")
	int32 PartySlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Pal")
	FTransform CapturedWorldTransform = FTransform::Identity;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPWCapturedPalsChanged);

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerPalStorageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerPalStorageComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Player|Pal")
	bool RegisterCapturedPal(APWPalBase* CapturedPal);

	UFUNCTION(BlueprintPure, Category = "Player|Pal")
	int32 GetCapturedPalCount() const { return CapturedPals.Num(); }

	UFUNCTION(BlueprintPure, Category = "Player|Pal")
	void GetCapturedPals(TArray<FPWCapturedPalRecord>& OutCapturedPals) const { OutCapturedPals = CapturedPals; }

	UFUNCTION(BlueprintCallable, Category = "Player|Pal")
	bool ToggleSummonPartyPal(int32 PartySlotIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Player|Pal")
	bool ToggleSummonSelectedPartyPal();

	UFUNCTION(BlueprintCallable, Category = "Player|Pal")
	bool SelectPreviousPartyPal();

	UFUNCTION(BlueprintCallable, Category = "Player|Pal")
	bool SelectNextPartyPal();

	UFUNCTION(BlueprintPure, Category = "Player|Pal")
	int32 GetSelectedPartySlotIndex() const { return SelectedPartySlotIndex; }

	UFUNCTION(BlueprintPure, Category = "Player|Pal")
	APWPalBase* GetActiveSummonedPal() const { return ActiveSummonedPal; }

	UPROPERTY(BlueprintAssignable, Category = "Player|Pal")
	FPWCapturedPalsChanged OnCapturedPalsChanged;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Player|Pal", meta = (ClampMin = "0"))
	int32 MaxPartySlots = 5;

	UPROPERTY(ReplicatedUsing = OnRep_CapturedPals)
	TArray<FPWCapturedPalRecord> CapturedPals;

	UPROPERTY(Replicated)
	TObjectPtr<APWPalBase> ActiveSummonedPal;

	UPROPERTY(Replicated)
	int32 ActiveSummonedPartySlotIndex = INDEX_NONE;

	UPROPERTY(ReplicatedUsing = OnRep_SelectedPartySlotIndex)
	int32 SelectedPartySlotIndex = 0;

	UFUNCTION()
	void OnRep_CapturedPals();

	UFUNCTION()
	void OnRep_SelectedPartySlotIndex();

	UFUNCTION(Server, Reliable)
	void ServerToggleSummonPartyPal(int32 PartySlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerToggleSummonSelectedPartyPal();

	UFUNCTION(Server, Reliable)
	void ServerSelectPartyPal(int32 Direction);

	FPWCapturedPalRecord MakeCapturedPalRecord(APWPalBase* CapturedPal) const;
	FName ResolvePalId(APWPalBase* CapturedPal) const;
	FString ResolveDisplayName(APWPalBase* CapturedPal) const;
	bool SelectPartyPalAuthority(int32 Direction);
	bool SummonPartyPalAuthority(int32 PartySlotIndex);
	bool RecallActivePalAuthority();
	FPWCapturedPalRecord* FindPartyRecord(int32 PartySlotIndex);
	const FPWCapturedPalRecord* FindPartyRecord(int32 PartySlotIndex) const;
	bool HasPartyRecord(int32 PartySlotIndex) const;
	int32 FindNextPartySlotIndex(int32 StartSlotIndex, int32 Direction) const;
	FTransform ResolveSummonTransform() const;
	void ApplyRecordToSummonedPal(APWPalBase* SummonedPal, const FPWCapturedPalRecord& Record) const;
	void SaveActivePalStateToRecord();
};
