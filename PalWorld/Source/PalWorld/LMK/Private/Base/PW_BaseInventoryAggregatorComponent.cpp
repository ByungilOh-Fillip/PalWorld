#include "Base/PW_BaseInventoryAggregatorComponent.h"

#include "Base/PW_StorageBoxActor.h"
#include "Base/PW_InventoryComponent.h"

UPW_BaseInventoryAggregatorComponent::UPW_BaseInventoryAggregatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UPW_BaseInventoryAggregatorComponent::RegisterStorageBox(APW_StorageBoxActor* StorageBox)
{
	if (GetOwner() != nullptr && GetOwner()->HasAuthority() && IsValid(StorageBox))
	{
		StorageBoxes.AddUnique(StorageBox);
	}
}

void UPW_BaseInventoryAggregatorComponent::UnregisterStorageBox(APW_StorageBoxActor* StorageBox)
{
	StorageBoxes.Remove(StorageBox);
}

int32 UPW_BaseInventoryAggregatorComponent::GetTotalItemCount(FName ItemId) const
{
	int32 Count = 0;
	for (const APW_StorageBoxActor* StorageBox : StorageBoxes)
	{
		const UPW_InventoryComponent* InventoryComponent = IsValid(StorageBox) ? StorageBox->GetInventoryComponent() : nullptr;
		if (InventoryComponent != nullptr)
		{
			Count += InventoryComponent->GetItemCount(ItemId);
		}
	}

	return Count;
}

bool UPW_BaseInventoryAggregatorComponent::AddItemToAnyStorage(FName ItemId, int32 Quantity)
{
	if (GetOwner() == nullptr || !GetOwner()->HasAuthority() || ItemId.IsNone() || Quantity <= 0)
	{
		return false;
	}

	CleanupInvalidStorageBoxes();

	for (APW_StorageBoxActor* StorageBox : StorageBoxes)
	{
		UPW_InventoryComponent* InventoryComponent = IsValid(StorageBox) ? StorageBox->GetInventoryComponent() : nullptr;
		if (InventoryComponent != nullptr && InventoryComponent->AddItem(ItemId, Quantity))
		{
			return true;
		}
	}

	return false;
}

bool UPW_BaseInventoryAggregatorComponent::ConsumeItems(FName ItemId, int32 Quantity)
{
	if (GetOwner() == nullptr || !GetOwner()->HasAuthority() || ItemId.IsNone() || Quantity <= 0)
	{
		return false;
	}

	CleanupInvalidStorageBoxes();

	if (GetTotalItemCount(ItemId) < Quantity)
	{
		return false;
	}

	int32 RemainingQuantity = Quantity;
	for (APW_StorageBoxActor* StorageBox : StorageBoxes)
	{
		UPW_InventoryComponent* InventoryComponent = IsValid(StorageBox) ? StorageBox->GetInventoryComponent() : nullptr;
		if (InventoryComponent == nullptr)
		{
			continue;
		}

		const int32 AvailableCount = InventoryComponent->GetItemCount(ItemId);
		const int32 RemoveCount = FMath::Min(AvailableCount, RemainingQuantity);
		if (RemoveCount > 0)
		{
			InventoryComponent->RemoveItem(ItemId, RemoveCount);
			RemainingQuantity -= RemoveCount;
		}

		if (RemainingQuantity <= 0)
		{
			return true;
		}
	}

	return false;
}

void UPW_BaseInventoryAggregatorComponent::CleanupInvalidStorageBoxes()
{
	StorageBoxes.RemoveAll([](const TObjectPtr<APW_StorageBoxActor>& StorageBox)
	{
		return !IsValid(StorageBox);
	});
}
