// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWPlayerInventoryLinkComponent.generated.h"

class UPWItemDataAsset;

USTRUCT(BlueprintType)
struct FPWInventoryItemStack
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Player|Inventory", meta = (ClampMin = "0"))
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Inventory")
	FName ItemId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Inventory", meta = (ClampMin = "0"))
	int32 Count = 0;
};

USTRUCT(BlueprintType)
struct FPWInventorySlotView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Player|Inventory", meta = (ClampMin = "0"))
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Inventory")
	bool bOccupied = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Inventory")
	FName ItemId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Inventory", meta = (ClampMin = "0"))
	int32 Count = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Inventory")
	TObjectPtr<UPWItemDataAsset> ItemData = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPWInventoryChangedSignature);

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerInventoryLinkComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerInventoryLinkComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Player|Inventory")
	bool AddItem(FName ItemId, int32 Count);

	UFUNCTION(BlueprintPure, Category = "Player|Inventory")
	int32 GetItemCount(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Player|Inventory")
	TArray<FPWInventoryItemStack> GetItems() const { return Items; }

	UFUNCTION(BlueprintPure, Category = "Player|Inventory")
	int32 GetInventorySlotCount() const { return InventorySlotCount; }

	UFUNCTION(BlueprintPure, Category = "Player|Inventory")
	FPWInventorySlotView GetSlotView(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Player|Inventory")
	TArray<FPWInventorySlotView> GetSlotViews() const;

	UFUNCTION(BlueprintPure, Category = "Player|Inventory")
	UPWItemDataAsset* GetItemDefinition(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Player|Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintPure, Category = "Player|Inventory")
	float GetMaxCarryWeight() const { return MaxCarryWeight; }

	UFUNCTION(BlueprintPure, Category = "Player|Inventory")
	bool IsInventoryFull() const;

	UPROPERTY(BlueprintAssignable, Category = "Player|Inventory")
	FPWInventoryChangedSignature OnInventoryChanged;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Player|Inventory", meta = (ClampMin = "1"))
	int32 InventorySlotCount = 40;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Inventory", meta = (ClampMin = "0.0"))
	float MaxCarryWeight = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Inventory|Data", meta = (TitleProperty = "ItemId"))
	TArray<TObjectPtr<UPWItemDataAsset>> ItemDefinitions;

	UPROPERTY(ReplicatedUsing = OnRep_Items, VisibleInstanceOnly, Category = "Player|Inventory")
	TArray<FPWInventoryItemStack> Items;

	UFUNCTION(Server, Reliable)
	void ServerAddItem(FName ItemId, int32 Count);

	UFUNCTION()
	void OnRep_Items();

	bool AddItemAuthority(FName ItemId, int32 Count);
	FPWInventoryItemStack* FindStack(FName ItemId);
	const FPWInventoryItemStack* FindStack(FName ItemId) const;
	FPWInventoryItemStack* FindStackBySlot(int32 SlotIndex);
	const FPWInventoryItemStack* FindStackBySlot(int32 SlotIndex) const;
	int32 FindFirstEmptySlotIndex() const;
	int32 GetMaxStackForItem(FName ItemId) const;
};
