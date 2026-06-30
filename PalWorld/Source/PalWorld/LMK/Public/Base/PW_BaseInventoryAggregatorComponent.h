#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BaseTypes.h"
#include "Components/ActorComponent.h"
#include "PW_BaseInventoryAggregatorComponent.generated.h"

class APW_StorageBoxActor;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_BaseInventoryAggregatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_BaseInventoryAggregatorComponent();

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Inventory")
	void RegisterStorageBox(APW_StorageBoxActor* StorageBox);

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Inventory")
	void UnregisterStorageBox(APW_StorageBoxActor* StorageBox);

	UFUNCTION(BlueprintPure, Category = "PW|Base|Inventory")
	int32 GetTotalItemCount(FName ItemId) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Inventory")
	bool AddItemToAnyStorage(FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Inventory")
	bool ConsumeItems(FName ItemId, int32 Quantity);

private:
	UPROPERTY()
	TArray<TObjectPtr<APW_StorageBoxActor>> StorageBoxes;

	void CleanupInvalidStorageBoxes();
};
