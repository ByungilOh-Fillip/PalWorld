// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWPlayerInventoryLinkComponent.generated.h"

class UPWItemDataAsset;
class UPWPlayerEquipmentComponent;

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

USTRUCT(BlueprintType)
struct FPWInventoryStarterItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Inventory")
	TObjectPtr<UPWItemDataAsset> ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Inventory", meta = (ClampMin = "1"))
	int32 Count = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPWInventoryChangedSignature);

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerInventoryLinkComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerInventoryLinkComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Player|Inventory")
	bool AddItem(FName ItemId, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Player|Inventory")
	bool MoveItemSlot(int32 FromSlotIndex, int32 ToSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Player|Inventory")
	bool DropItemFromSlot(int32 SlotIndex, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Player|Inventory")
	bool DestroyItemFromSlot(int32 SlotIndex, int32 Count);

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
	int32 InventorySlotCount = 42;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Inventory", meta = (ClampMin = "0.0"))
	float MaxCarryWeight = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Inventory|Data", meta = (TitleProperty = "ItemId"))
	TArray<TObjectPtr<UPWItemDataAsset>> ItemDefinitions;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Inventory|Data", meta = (TitleProperty = "ItemData"))
	TArray<FPWInventoryStarterItem> StarterItems;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Inventory|Debug")
	bool bAutoRegisterDefaultItemDefinitions = true;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Inventory|Debug")
	bool bGrantDefaultStarterItems = true;

	UPROPERTY(ReplicatedUsing = OnRep_Items, VisibleInstanceOnly, Category = "Player|Inventory")
	TArray<FPWInventoryItemStack> Items;

	UFUNCTION(Server, Reliable)
	void ServerAddItem(FName ItemId, int32 Count);

	UFUNCTION(Server, Reliable)
	void ServerMoveItemSlot(int32 FromSlotIndex, int32 ToSlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerDropItemFromSlot(int32 SlotIndex, int32 Count);

	UFUNCTION(Server, Reliable)
	void ServerDestroyItemFromSlot(int32 SlotIndex, int32 Count);

	UFUNCTION()
	void OnRep_Items();

	friend class UPWPlayerEquipmentComponent;

	bool AddItemAuthority(FName ItemId, int32 Count);
	bool AddItemToSlotAuthority(FName ItemId, int32 Count, int32 TargetSlotIndex);
	bool MoveItemSlotAuthority(int32 FromSlotIndex, int32 ToSlotIndex);
	bool RemoveItemFromSlotAuthority(int32 SlotIndex, int32 Count, FPWInventoryItemStack* OutRemovedStack = nullptr);
	bool DropItemStackToWorldAuthority(const FPWInventoryItemStack& ItemStack);
	void EnsureDefaultItemDefinitions();
	void GrantStarterItemsAuthority();
	FName NormalizeItemId(FName ItemId) const;
	FPWInventoryItemStack* FindStack(FName ItemId);
	const FPWInventoryItemStack* FindStack(FName ItemId) const;
	FPWInventoryItemStack* FindStackBySlot(int32 SlotIndex);
	const FPWInventoryItemStack* FindStackBySlot(int32 SlotIndex) const;
	int32 FindFirstEmptySlotIndex() const;
	int32 GetMaxStackForItem(FName ItemId) const;
};
