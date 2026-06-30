#include "Base/PW_InventoryComponent.h"

#include "Net/UnrealNetwork.h"

UPW_InventoryComponent::UPW_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPW_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPW_InventoryComponent, Items);
}

bool UPW_InventoryComponent::AddItem(FName ItemId, int32 Quantity)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || ItemId.IsNone() || Quantity <= 0)
	{
		return false;
	}

	for (FPW_ItemStack& Stack : Items)
	{
		if (Stack.ItemId == ItemId)
		{
			Stack.Quantity += Quantity;
			Owner->ForceNetUpdate();
			return true;
		}
	}

	FPW_ItemStack NewStack;
	NewStack.ItemId = ItemId;
	NewStack.Quantity = Quantity;
	Items.Add(NewStack);
	Owner->ForceNetUpdate();
	return true;
}

bool UPW_InventoryComponent::RemoveItem(FName ItemId, int32 Quantity)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || ItemId.IsNone() || Quantity <= 0)
	{
		return false;
	}

	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		FPW_ItemStack& Stack = Items[Index];
		if (Stack.ItemId != ItemId || Stack.Quantity < Quantity)
		{
			continue;
		}

		Stack.Quantity -= Quantity;
		if (Stack.Quantity <= 0)
		{
			Items.RemoveAt(Index);
		}

		Owner->ForceNetUpdate();
		return true;
	}

	return false;
}

int32 UPW_InventoryComponent::GetItemCount(FName ItemId) const
{
	if (ItemId.IsNone())
	{
		return 0;
	}

	int32 Count = 0;
	for (const FPW_ItemStack& Stack : Items)
	{
		if (Stack.ItemId == ItemId)
		{
			Count += Stack.Quantity;
		}
	}

	return Count;
}
