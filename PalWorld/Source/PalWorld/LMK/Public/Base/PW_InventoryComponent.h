#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BaseTypes.h"
#include "Components/ActorComponent.h"
#include "PW_InventoryComponent.generated.h"

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_InventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "PW|Inventory")
	bool AddItem(FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "PW|Inventory")
	bool RemoveItem(FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintPure, Category = "PW|Inventory")
	int32 GetItemCount(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "PW|Inventory")
	const TArray<FPW_ItemStack>& GetItems() const { return Items; }

protected:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "PW|Inventory")
	TArray<FPW_ItemStack> Items;
};
