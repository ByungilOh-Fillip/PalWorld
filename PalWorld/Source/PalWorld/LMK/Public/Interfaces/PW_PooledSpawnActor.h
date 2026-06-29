#pragma once

#include "CoreMinimal.h"
#include "Spawn/PW_WildPalSpawner.h"
#include "UObject/Interface.h"
#include "PW_PooledSpawnActor.generated.h"

UINTERFACE(BlueprintType)
class PALWORLD_API UPW_PooledSpawnActor : public UInterface
{
	GENERATED_BODY()
};

class PALWORLD_API IPW_PooledSpawnActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Spawn Pool")
	void OnActivatedFromPool(APW_WildPalSpawner* OwningSpawner);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Spawn Pool")
	void OnReturnedToPool(APW_WildPalSpawner* OwningSpawner);
};
