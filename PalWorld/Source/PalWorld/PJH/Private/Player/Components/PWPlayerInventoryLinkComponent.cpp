// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerInventoryLinkComponent.h"

#include "Net/UnrealNetwork.h"
#include "Player/Data/PWItemDataAsset.h"

UPWPlayerInventoryLinkComponent::UPWPlayerInventoryLinkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPWPlayerInventoryLinkComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureDefaultItemDefinitions();

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority() || Items.Num() > 0)
	{
		return;
	}

	GrantStarterItemsAuthority();
}

void UPWPlayerInventoryLinkComponent::EnsureDefaultItemDefinitions()
{
	if (!bAutoRegisterDefaultItemDefinitions)
	{
		return;
	}

	static const TCHAR* DefaultItemPaths[] = {
		TEXT("/Game/PJH/Data/Items/DA_Item_Wood.DA_Item_Wood"),
		TEXT("/Game/PJH/Data/Items/DA_Item_Stone.DA_Item_Stone"),
		TEXT("/Game/PJH/Data/Items/DA_Item_Pickaxe.DA_Item_Pickaxe"),
		TEXT("/Game/PJH/Data/Items/DA_Item_Axe.DA_Item_Axe"),
		TEXT("/Game/PJH/Data/Items/DA_Item_PalSphere.DA_Item_PalSphere"),
		TEXT("/Game/PJH/Data/Items/DA_Item_TestHelmet.DA_Item_TestHelmet"),
		TEXT("/Game/PJH/Data/Items/DA_Item_TestArmor.DA_Item_TestArmor"),
		TEXT("/Game/PJH/Data/Items/DA_Item_TestShield.DA_Item_TestShield"),
		TEXT("/Game/PJH/Data/Items/DA_Item_TestGlider.DA_Item_TestGlider"),
		TEXT("/Game/PJH/Data/Items/DA_Item_TestSphereModule.DA_Item_TestSphereModule"),
		TEXT("/Game/PJH/Data/Items/DA_Item_TestAccessory.DA_Item_TestAccessory"),
		TEXT("/Game/PJH/Data/Items/DA_Item_TestFood.DA_Item_TestFood")
	};

	for (const TCHAR* DefaultItemPath : DefaultItemPaths)
	{
		UPWItemDataAsset* ItemData = LoadObject<UPWItemDataAsset>(nullptr, DefaultItemPath);
		if (!ItemData)
		{
			continue;
		}

		const bool bAlreadyRegistered = ItemDefinitions.ContainsByPredicate(
			[ItemData](const UPWItemDataAsset* ExistingItemData)
			{
				return ExistingItemData == ItemData
					|| (ExistingItemData && ExistingItemData->GetItemId() == ItemData->GetItemId());
			});

		if (!bAlreadyRegistered)
		{
			ItemDefinitions.Add(ItemData);
		}
	}
}

void UPWPlayerInventoryLinkComponent::GrantStarterItemsAuthority()
{
	for (const FPWInventoryStarterItem& StarterItem : StarterItems)
	{
		if (StarterItem.ItemData && StarterItem.Count > 0)
		{
			AddItemAuthority(StarterItem.ItemData->GetItemId(), StarterItem.Count);
		}
	}

	if (!bGrantDefaultStarterItems || StarterItems.Num() > 0)
	{
		return;
	}

	struct FDefaultStarterItem
	{
		FName ItemId;
		int32 Count = 1;
	};

	const FDefaultStarterItem DefaultStarterItems[] = {
		{ TEXT("Wood"), 50 },
		{ TEXT("Stone"), 50 },
		{ TEXT("Pickaxe"), 1 },
		{ TEXT("Axe"), 1 },
		{ TEXT("PalSphere"), 10 },
		{ TEXT("TestHelmet"), 1 },
		{ TEXT("TestArmor"), 1 },
		{ TEXT("TestShield"), 1 },
		{ TEXT("TestGlider"), 1 },
		{ TEXT("TestSphereModule"), 1 },
		{ TEXT("TestAccessory"), 1 },
		{ TEXT("TestFood"), 10 }
	};

	for (const FDefaultStarterItem& DefaultStarterItem : DefaultStarterItems)
	{
		AddItemAuthority(DefaultStarterItem.ItemId, DefaultStarterItem.Count);
	}
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

bool UPWPlayerInventoryLinkComponent::MoveItemSlot(int32 FromSlotIndex, int32 ToSlotIndex)
{
	if (FromSlotIndex == ToSlotIndex)
	{
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		ServerMoveItemSlot(FromSlotIndex, ToSlotIndex);
		return true;
	}

	return MoveItemSlotAuthority(FromSlotIndex, ToSlotIndex);
}

bool UPWPlayerInventoryLinkComponent::DropItemFromSlot(int32 SlotIndex, int32 Count)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		ServerDropItemFromSlot(SlotIndex, Count);
		return true;
	}

	return RemoveItemFromSlotAuthority(SlotIndex, Count);
}

bool UPWPlayerInventoryLinkComponent::DestroyItemFromSlot(int32 SlotIndex, int32 Count)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		ServerDestroyItemFromSlot(SlotIndex, Count);
		return true;
	}

	return RemoveItemFromSlotAuthority(SlotIndex, Count);
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

void UPWPlayerInventoryLinkComponent::ServerMoveItemSlot_Implementation(int32 FromSlotIndex, int32 ToSlotIndex)
{
	MoveItemSlotAuthority(FromSlotIndex, ToSlotIndex);
}

void UPWPlayerInventoryLinkComponent::ServerDropItemFromSlot_Implementation(int32 SlotIndex, int32 Count)
{
	RemoveItemFromSlotAuthority(SlotIndex, Count);
}

void UPWPlayerInventoryLinkComponent::ServerDestroyItemFromSlot_Implementation(int32 SlotIndex, int32 Count)
{
	RemoveItemFromSlotAuthority(SlotIndex, Count);
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

	OnInventoryChanged.Broadcast();
	OwnerActor->ForceNetUpdate();
	return RemainingCount <= 0;
}

bool UPWPlayerInventoryLinkComponent::MoveItemSlotAuthority(int32 FromSlotIndex, int32 ToSlotIndex)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()
		|| FromSlotIndex == ToSlotIndex
		|| FromSlotIndex < 0 || FromSlotIndex >= InventorySlotCount
		|| ToSlotIndex < 0 || ToSlotIndex >= InventorySlotCount)
	{
		return false;
	}

	FPWInventoryItemStack* FromStack = FindStackBySlot(FromSlotIndex);
	if (!FromStack || FromStack->ItemId.IsNone() || FromStack->Count <= 0)
	{
		return false;
	}

	FPWInventoryItemStack* ToStack = FindStackBySlot(ToSlotIndex);
	if (!ToStack)
	{
		FromStack->SlotIndex = ToSlotIndex;
		OnInventoryChanged.Broadcast();
		OwnerActor->ForceNetUpdate();
		return true;
	}

	if (FromStack->ItemId == ToStack->ItemId)
	{
		const int32 MaxStack = GetMaxStackForItem(FromStack->ItemId);
		const int32 MoveCount = FMath::Min(FromStack->Count, MaxStack - ToStack->Count);
		if (MoveCount > 0)
		{
			ToStack->Count += MoveCount;
			FromStack->Count -= MoveCount;
			if (FromStack->Count <= 0)
			{
				Items.RemoveAll(
					[FromSlotIndex](const FPWInventoryItemStack& Stack)
					{
						return Stack.SlotIndex == FromSlotIndex;
					});
			}

			OnInventoryChanged.Broadcast();
			OwnerActor->ForceNetUpdate();
			return true;
		}
	}

	Swap(FromStack->SlotIndex, ToStack->SlotIndex);
	OnInventoryChanged.Broadcast();
	OwnerActor->ForceNetUpdate();
	return true;
}

bool UPWPlayerInventoryLinkComponent::RemoveItemFromSlotAuthority(int32 SlotIndex, int32 Count, FPWInventoryItemStack* OutRemovedStack)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()
		|| SlotIndex < 0 || SlotIndex >= InventorySlotCount)
	{
		return false;
	}

	FPWInventoryItemStack* Stack = FindStackBySlot(SlotIndex);
	if (!Stack || Stack->ItemId.IsNone() || Stack->Count <= 0)
	{
		return false;
	}

	const int32 RemoveCount = Count <= 0 ? Stack->Count : FMath::Min(Count, Stack->Count);
	if (RemoveCount <= 0)
	{
		return false;
	}

	if (OutRemovedStack)
	{
		OutRemovedStack->SlotIndex = SlotIndex;
		OutRemovedStack->ItemId = Stack->ItemId;
		OutRemovedStack->Count = RemoveCount;
	}

	Stack->Count -= RemoveCount;
	if (Stack->Count <= 0)
	{
		Items.RemoveAll(
			[SlotIndex](const FPWInventoryItemStack& Candidate)
			{
				return Candidate.SlotIndex == SlotIndex;
			});
	}

	OnInventoryChanged.Broadcast();
	OwnerActor->ForceNetUpdate();
	return true;
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
