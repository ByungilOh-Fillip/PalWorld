#pragma once

#include "CoreMinimal.h"
#include "PWPalBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "PW_PalSpawnPoolSubsystem.generated.h"

class APW_WildPalSpawner;

USTRUCT()
struct FPW_PooledPalActorList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<APWPalBase>> Actors;
};

UCLASS()
class PALWORLD_API UPW_PalSpawnPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PW|Spawn Pool")
	APWPalBase* AcquirePal(TSubclassOf<APWPalBase> PalClass, const FTransform& SpawnTransform, APW_WildPalSpawner* OwningSpawner);

	UFUNCTION(BlueprintCallable, Category = "PW|Spawn Pool")
	void ReturnPal(APWPalBase* PalActor, APW_WildPalSpawner* OwningSpawner);

	UFUNCTION(BlueprintCallable, Category = "PW|Spawn Pool")
	int32 GetPooledActorCount() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "PW|Spawn Pool", meta = (ClampMin = "0"))
	int32 MaxPooledActors = 70;

private:
	UPROPERTY()
	TMap<TSubclassOf<APWPalBase>, FPW_PooledPalActorList> PooledActorsByClass;

	bool CanRunAuthorityPool() const;
	void ActivateActor(APWPalBase* PalActor, const FTransform& SpawnTransform, APW_WildPalSpawner* OwningSpawner) const;
	void DeactivateActor(APWPalBase* PalActor, APW_WildPalSpawner* OwningSpawner) const;
	void TrimPoolIfNeeded();
};
