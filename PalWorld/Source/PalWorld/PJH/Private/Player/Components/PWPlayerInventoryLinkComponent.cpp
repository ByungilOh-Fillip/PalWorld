// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerInventoryLinkComponent.h"

#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPlayerStatComponent.h"
#include "Player/Data/PWItemDataAsset.h"
#include "World/PWWorldItemDropLibrary.h"

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
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	if (Items.Num() <= 0)
	{
		GrantStarterItemsAuthority();
	}
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
		TEXT("/Game/PJH/Data/Items/DA_Item_Helmet.DA_Item_Helmet"),
		TEXT("/Game/PJH/Data/Items/DA_Item_Armor.DA_Item_Armor"),
		TEXT("/Game/PJH/Data/Items/DA_Item_Shield.DA_Item_Shield"),
		TEXT("/Game/PJH/Data/Items/DA_Item_Glider.DA_Item_Glider"),
		TEXT("/Game/PJH/Data/Items/DA_Item_Accessory.DA_Item_Accessory"),
		TEXT("/Game/PJH/Data/Items/DA_Item_SphereModule.DA_Item_SphereModule"),
		TEXT("/Game/PJH/Data/Items/DA_Item_PalSphere.DA_Item_PalSphere"),
		TEXT("/Game/PJH/Data/Items/DA_Item_Food.DA_Item_Food")
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

	if (!bGrantDefaultStarterItems)
	{
		return;
	}

	struct FDefaultStarterItem
	{
		const TCHAR* ItemPath = nullptr;
		int32 Count = 1;
	};

	const FDefaultStarterItem DefaultStarterItems[] = {
		{ TEXT("/Game/PJH/Data/Items/DA_Item_Wood.DA_Item_Wood"), 50 },
		{ TEXT("/Game/PJH/Data/Items/DA_Item_Stone.DA_Item_Stone"), 50 },
		{ TEXT("/Game/PJH/Data/Items/DA_Item_Pickaxe.DA_Item_Pickaxe"), 1 },
		{ TEXT("/Game/PJH/Data/Items/DA_Item_Axe.DA_Item_Axe"), 1 },
		{ TEXT("/Game/PJH/Data/Items/DA_Item_Helmet.DA_Item_Helmet"), 1 },
		{ TEXT("/Game/PJH/Data/Items/DA_Item_Armor.DA_Item_Armor"), 1 },
		{ TEXT("/Game/PJH/Data/Items/DA_Item_Shield.DA_Item_Shield"), 1 },
		{ TEXT("/Game/PJH/Data/Items/DA_Item_Glider.DA_Item_Glider"), 1 },
		{ TEXT("/Game/PJH/Data/Items/DA_Item_Accessory.DA_Item_Accessory"), 1 },
		{ TEXT("/Game/PJH/Data/Items/DA_Item_SphereModule.DA_Item_SphereModule"), 1 },
		{ TEXT("/Game/PJH/Data/Items/DA_Item_PalSphere.DA_Item_PalSphere"), 10 },
		{ TEXT("/Game/PJH/Data/Items/DA_Item_Food.DA_Item_Food"), 10 }
	};

	for (const FDefaultStarterItem& DefaultStarterItem : DefaultStarterItems)
	{
		UPWItemDataAsset* ItemData = LoadObject<UPWItemDataAsset>(nullptr, DefaultStarterItem.ItemPath);
		if (!ItemData)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PWInventory] Failed to load default starter item. Path=%s"), DefaultStarterItem.ItemPath);
			continue;
		}

		AddItemAuthority(ItemData->GetItemId(), DefaultStarterItem.Count);
	}
}

void UPWPlayerInventoryLinkComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPWPlayerInventoryLinkComponent, Items);
}

bool UPWPlayerInventoryLinkComponent::AddItem(FName ItemId, int32 Count)
{
	ItemId = NormalizeItemId(ItemId);
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

	const FPWInventoryItemStack* SourceStack = FindStackBySlot(SlotIndex);
	if (!SourceStack || SourceStack->ItemId.IsNone() || SourceStack->Count <= 0)
	{
		return false;
	}

	const int32 DropCount = Count <= 0 ? SourceStack->Count : FMath::Min(Count, SourceStack->Count);
	UPWItemDataAsset* ItemData = GetItemDefinition(SourceStack->ItemId);
	if (!UPWWorldItemDropLibrary::CanSpawnWorldItem(ItemData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWInventory] Drop rejected. Item has no world mesh. Slot=%d Item=%s ItemData=%s"),
			SlotIndex,
			*SourceStack->ItemId.ToString(),
			*GetNameSafe(ItemData));
		return false;
	}

	FPWInventoryItemStack RemovedStack;
	if (!RemoveItemFromSlotAuthority(SlotIndex, DropCount, &RemovedStack))
	{
		return false;
	}

	if (DropItemStackToWorldAuthority(RemovedStack))
	{
		return true;
	}

	// 월드 스폰에 실패하면 아이템이 증발하지 않도록 원래 슬롯 복구를 먼저 시도한다.
	if (!AddItemToSlotAuthority(RemovedStack.ItemId, RemovedStack.Count, RemovedStack.SlotIndex))
	{
		AddItemAuthority(RemovedStack.ItemId, RemovedStack.Count);
	}
	return false;
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

bool UPWPlayerInventoryLinkComponent::UseItemFromSlot(int32 SlotIndex)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		ServerUseItemFromSlot(SlotIndex);
		return true;
	}

	return UseItemFromSlotAuthority(SlotIndex);
}

int32 UPWPlayerInventoryLinkComponent::GetItemCount(FName ItemId) const
{
	ItemId = NormalizeItemId(ItemId);

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
	ItemId = NormalizeItemId(ItemId);
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
	ItemId = NormalizeItemId(ItemId);
	AddItemAuthority(ItemId, Count);
}

void UPWPlayerInventoryLinkComponent::ServerMoveItemSlot_Implementation(int32 FromSlotIndex, int32 ToSlotIndex)
{
	MoveItemSlotAuthority(FromSlotIndex, ToSlotIndex);
}

void UPWPlayerInventoryLinkComponent::ServerDropItemFromSlot_Implementation(int32 SlotIndex, int32 Count)
{
	DropItemFromSlot(SlotIndex, Count);
}

void UPWPlayerInventoryLinkComponent::ServerDestroyItemFromSlot_Implementation(int32 SlotIndex, int32 Count)
{
	RemoveItemFromSlotAuthority(SlotIndex, Count);
}

void UPWPlayerInventoryLinkComponent::ServerUseItemFromSlot_Implementation(int32 SlotIndex)
{
	UseItemFromSlotAuthority(SlotIndex);
}

void UPWPlayerInventoryLinkComponent::OnRep_Items()
{
	OnInventoryChanged.Broadcast();
}

bool UPWPlayerInventoryLinkComponent::AddItemAuthority(FName ItemId, int32 Count)
{
	ItemId = NormalizeItemId(ItemId);
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
		return false;
	}

	OnInventoryChanged.Broadcast();
	OwnerActor->ForceNetUpdate();
	return RemainingCount <= 0;
}

bool UPWPlayerInventoryLinkComponent::AddItemToSlotAuthority(FName ItemId, int32 Count, int32 TargetSlotIndex)
{
	ItemId = NormalizeItemId(ItemId);
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()
		|| ItemId.IsNone()
		|| Count <= 0
		|| TargetSlotIndex < 0
		|| TargetSlotIndex >= InventorySlotCount)
	{
		return false;
	}

	FPWInventoryItemStack* TargetStack = FindStackBySlot(TargetSlotIndex);
	if (!TargetStack)
	{
		const int32 MaxStack = GetMaxStackForItem(ItemId);
		if (Count > MaxStack)
		{
			return false;
		}

		FPWInventoryItemStack NewStack;
		NewStack.SlotIndex = TargetSlotIndex;
		NewStack.ItemId = ItemId;
		NewStack.Count = Count;
		Items.Add(NewStack);

		OnInventoryChanged.Broadcast();
		OwnerActor->ForceNetUpdate();
		return true;
	}

	if (TargetStack->ItemId != ItemId)
	{
		return false;
	}

	const int32 MaxStack = GetMaxStackForItem(ItemId);
	if (TargetStack->Count + Count > MaxStack)
	{
		return false;
	}

	TargetStack->Count += Count;
	OnInventoryChanged.Broadcast();
	OwnerActor->ForceNetUpdate();
	return true;
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

bool UPWPlayerInventoryLinkComponent::UseItemFromSlotAuthority(int32 SlotIndex)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()
		|| SlotIndex < 0 || SlotIndex >= InventorySlotCount)
	{
		return false;
	}

	const FPWInventoryItemStack* Stack = FindStackBySlot(SlotIndex);
	if (!Stack || Stack->ItemId.IsNone() || Stack->Count <= 0)
	{
		return false;
	}

	UPWItemDataAsset* ItemData = GetItemDefinition(Stack->ItemId);
	if (!ItemData || ItemData->GetItemType() != EPWItemType::Consumable)
	{
		return false;
	}

	if (!ApplyConsumableItemAuthority(ItemData))
	{
		return false;
	}

	return RemoveItemFromSlotAuthority(SlotIndex, 1);
}

bool UPWPlayerInventoryLinkComponent::ApplyConsumableItemAuthority(UPWItemDataAsset* ItemData)
{
	AActor* OwnerActor = GetOwner();
	UPWPlayerStatComponent* StatComponent = OwnerActor ? OwnerActor->FindComponentByClass<UPWPlayerStatComponent>() : nullptr;
	if (!OwnerActor || !OwnerActor->HasAuthority() || !ItemData || !StatComponent)
	{
		return false;
	}

	bool bAppliedAnyEffect = false;
	const float HungerRestoreAmount = ItemData->GetHungerRestoreAmount();
	if (HungerRestoreAmount > 0.f && StatComponent->GetCurrentHunger() < StatComponent->GetMaxHunger())
	{
		bAppliedAnyEffect |= StatComponent->RestoreHunger(HungerRestoreAmount);
	}

	const float HealthRestoreAmount = ItemData->GetHealthRestoreAmount();
	if (HealthRestoreAmount > 0.f && StatComponent->GetCurrentHealth() < StatComponent->GetMaxHealth())
	{
		bAppliedAnyEffect |= StatComponent->RestoreHealth(HealthRestoreAmount);
	}

	return bAppliedAnyEffect;
}

bool UPWPlayerInventoryLinkComponent::DropItemStackToWorldAuthority(const FPWInventoryItemStack& ItemStack)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority() || ItemStack.ItemId.IsNone() || ItemStack.Count <= 0)
	{
		return false;
	}

	UPWItemDataAsset* ItemData = GetItemDefinition(ItemStack.ItemId);
	if (!UPWWorldItemDropLibrary::CanSpawnWorldItem(ItemData))
	{
		return false;
	}

	const FVector Forward = OwnerActor->GetActorForwardVector();
	FPWWorldItemDropRequest DropRequest;
	DropRequest.ItemData = ItemData;
	DropRequest.ItemId = ItemStack.ItemId;
	DropRequest.Count = ItemStack.Count;
	DropRequest.SourceActor = OwnerActor;
	DropRequest.SourceLocation = OwnerActor->GetActorLocation() + Forward * 120.f + FVector(0.f, 0.f, 45.f);
	DropRequest.TargetLocation = OwnerActor->GetActorLocation() + Forward * 260.f;
	DropRequest.TowardTargetMinAlpha = 0.75f;
	DropRequest.TowardTargetMaxAlpha = 1.f;
	DropRequest.ScatterRadius = 35.f;
	DropRequest.bIgnoreSourceActorInGroundTrace = true;
	DropRequest.MinHorizontalImpulse = 90.f;
	DropRequest.MaxHorizontalImpulse = 160.f;
	DropRequest.MinUpwardImpulse = 140.f;
	DropRequest.MaxUpwardImpulse = 230.f;
	DropRequest.bStartAutoCollect = false;

	return UPWWorldItemDropLibrary::SpawnWorldItemDrop(this, DropRequest) != nullptr;
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
	ItemId = NormalizeItemId(ItemId);
	const UPWItemDataAsset* ItemDefinition = GetItemDefinition(ItemId);
	return ItemDefinition ? FMath::Max(1, ItemDefinition->GetMaxStack()) : 999;
}

FName UPWPlayerInventoryLinkComponent::NormalizeItemId(FName ItemId) const
{
	if (ItemId.IsNone())
	{
		return NAME_None;
	}

	const FString ItemIdString = ItemId.ToString();
	if (ItemIdString.Contains(TEXT("Rock")) || ItemIdString.Contains(TEXT("Ore")))
	{
		return TEXT("Stone");
	}

	return ItemId;
}
