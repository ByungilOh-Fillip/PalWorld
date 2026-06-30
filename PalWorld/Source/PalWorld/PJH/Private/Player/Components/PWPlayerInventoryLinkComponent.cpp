// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerInventoryLinkComponent.h"

#include "Net/UnrealNetwork.h"
#include "Player/Data/PWItemDataAsset.h"

UPWPlayerInventoryLinkComponent::UPWPlayerInventoryLinkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPWPlayerInventoryLinkComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPWPlayerInventoryLinkComponent, Items);
}

bool UPWPlayerInventoryLinkComponent::AddItem(FName ItemId, int32 Count)
{
	if (ItemId.IsNone() || Count <= 0)
	{
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		ServerAddItem(ItemId, Count);
		return true;
	}

	return AddItemAuthority(ItemId, Count);
}

int32 UPWPlayerInventoryLinkComponent::GetItemCount(FName ItemId) const
{
	int32 TotalCount = 0;
	for (const FPWInventoryItemStack& Stack : Items)
	{
		if (Stack.ItemId == ItemId)
		{
			TotalCount += Stack.Count;
		}
	}

	return TotalCount;
}

FPWInventorySlotView UPWPlayerInventoryLinkComponent::GetSlotView(int32 SlotIndex) const
{
	FPWInventorySlotView SlotView;
	SlotView.SlotIndex = SlotIndex;

	const FPWInventoryItemStack* Stack = FindStackBySlot(SlotIndex);
	if (!Stack || Stack->ItemId.IsNone() || Stack->Count <= 0)
	{
		return SlotView;
	}

	SlotView.bOccupied = true;
	SlotView.ItemId = Stack->ItemId;
	SlotView.Count = Stack->Count;
	SlotView.ItemData = GetItemDefinition(Stack->ItemId);
	return SlotView;
}

TArray<FPWInventorySlotView> UPWPlayerInventoryLinkComponent::GetSlotViews() const
{
	TArray<FPWInventorySlotView> SlotViews;
	SlotViews.Reserve(InventorySlotCount);

	for (int32 SlotIndex = 0; SlotIndex < InventorySlotCount; ++SlotIndex)
	{
		SlotViews.Add(GetSlotView(SlotIndex));
	}

	return SlotViews;
}

UPWItemDataAsset* UPWPlayerInventoryLinkComponent::GetItemDefinition(FName ItemId) const
{
	if (ItemId.IsNone())
	{
		return nullptr;
	}

	for (UPWItemDataAsset* ItemDefinition : ItemDefinitions)
	{
		if (ItemDefinition && ItemDefinition->GetItemId() == ItemId)
		{
			return ItemDefinition;
		}
	}

	return nullptr;
}

float UPWPlayerInventoryLinkComponent::GetCurrentWeight() const
{
	float CurrentWeight = 0.f;
	for (const FPWInventoryItemStack& Stack : Items)
	{
		if (Stack.Count <= 0)
		{
			continue;
		}

		const UPWItemDataAsset* ItemDefinition = GetItemDefinition(Stack.ItemId);
		CurrentWeight += (ItemDefinition ? ItemDefinition->GetWeight() : 0.f) * Stack.Count;
	}

	return CurrentWeight;
}

bool UPWPlayerInventoryLinkComponent::IsInventoryFull() const
{
	if (FindFirstEmptySlotIndex() != INDEX_NONE)
	{
		return false;
	}

	for (const FPWInventoryItemStack& Stack : Items)
	{
		if (Stack.Count < GetMaxStackForItem(Stack.ItemId))
		{
			return false;
		}
	}

	return true;
}

void UPWPlayerInventoryLinkComponent::ServerAddItem_Implementation(FName ItemId, int32 Count)
{
	AddItemAuthority(ItemId, Count);
}

void UPWPlayerInventoryLinkComponent::OnRep_Items()
{
	OnInventoryChanged.Broadcast();
}

bool UPWPlayerInventoryLinkComponent::AddItemAuthority(FName ItemId, int32 Count)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority() || ItemId.IsNone() || Count <= 0)
	{
		return false;
	}

	const int32 MaxStack = GetMaxStackForItem(ItemId);
	int32 RemainingCount = Count;

	for (FPWInventoryItemStack& ExistingStack : Items)
	{
		if (ExistingStack.ItemId != ItemId || ExistingStack.Count >= MaxStack)
		{
			continue;
		}

		const int32 AddCount = FMath::Min(RemainingCount, MaxStack - ExistingStack.Count);
		ExistingStack.Count += AddCount;
		RemainingCount -= AddCount;

		if (RemainingCount <= 0)
		{
			break;
		}
	}

	while (RemainingCount > 0)
	{
		const int32 EmptySlotIndex = FindFirstEmptySlotIndex();
		if (EmptySlotIndex == INDEX_NONE)
		{
			break;
		}

		const int32 AddCount = FMath::Min(RemainingCount, MaxStack);
		FPWInventoryItemStack NewStack;
		NewStack.SlotIndex = EmptySlotIndex;
		NewStack.ItemId = ItemId;
		NewStack.Count = AddCount;
		Items.Add(NewStack);
		RemainingCount -= AddCount;
	}

	const int32 AddedCount = Count - RemainingCount;
	if (AddedCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWInventory] Inventory is full. Owner=%s Item=%s Count=%d"),
			*OwnerActor->GetName(),
			*ItemId.ToString(),
			Count);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[PWInventory] Added item. Owner=%s Item=%s Count=%d Total=%d"),
		*OwnerActor->GetName(),
		*ItemId.ToString(),
		AddedCount,
		GetItemCount(ItemId));

	OnInventoryChanged.Broadcast();
	OwnerActor->ForceNetUpdate();
	return RemainingCount <= 0;
}

FPWInventoryItemStack* UPWPlayerInventoryLinkComponent::FindStack(FName ItemId)
{
	return Items.FindByPredicate(
		[ItemId](const FPWInventoryItemStack& Stack)
		{
			return Stack.ItemId == ItemId;
		});
}

const FPWInventoryItemStack* UPWPlayerInventoryLinkComponent::FindStack(FName ItemId) const
{
	return Items.FindByPredicate(
		[ItemId](const FPWInventoryItemStack& Stack)
		{
			return Stack.ItemId == ItemId;
		});
}

FPWInventoryItemStack* UPWPlayerInventoryLinkComponent::FindStackBySlot(int32 SlotIndex)
{
	return Items.FindByPredicate(
		[SlotIndex](const FPWInventoryItemStack& Stack)
		{
			return Stack.SlotIndex == SlotIndex;
		});
}

const FPWInventoryItemStack* UPWPlayerInventoryLinkComponent::FindStackBySlot(int32 SlotIndex) const
{
	return Items.FindByPredicate(
		[SlotIndex](const FPWInventoryItemStack& Stack)
		{
			return Stack.SlotIndex == SlotIndex;
		});
}

int32 UPWPlayerInventoryLinkComponent::FindFirstEmptySlotIndex() const
{
	for (int32 SlotIndex = 0; SlotIndex < InventorySlotCount; ++SlotIndex)
	{
		if (!FindStackBySlot(SlotIndex))
		{
			return SlotIndex;
		}
	}

	return INDEX_NONE;
}

int32 UPWPlayerInventoryLinkComponent::GetMaxStackForItem(FName ItemId) const
{
	const UPWItemDataAsset* ItemDefinition = GetItemDefinition(ItemId);
	return ItemDefinition ? FMath::Max(1, ItemDefinition->GetMaxStack()) : 999;
}
