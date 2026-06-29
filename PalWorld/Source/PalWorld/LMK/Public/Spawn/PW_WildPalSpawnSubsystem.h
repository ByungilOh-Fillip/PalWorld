#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PW_WildPalSpawnSubsystem.generated.h"

class APW_WildPalSpawner;

UCLASS()
class PALWORLD_API UPW_WildPalSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	void RegisterSpawner(APW_WildPalSpawner* Spawner);
	void UnregisterSpawner(APW_WildPalSpawner* Spawner);

	bool TryReserveGlobalPal();
	void ReleaseGlobalPal();

	bool IsAnyPlayerWithin(const FVector& Origin, float Radius) const;
	bool AreAllPlayersOutside(const FVector& Origin, float Radius) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Wild Pal Spawn")
	int32 GetActivePalCount() const;

	UFUNCTION(BlueprintCallable, Category = "PW|Wild Pal Spawn")
	int32 GetMaxActivePalCount() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "PW|Wild Pal Spawn", meta = (ClampMin = "0"))
	int32 MaxActivePalCount = 70;

	UPROPERTY(EditDefaultsOnly, Category = "PW|Wild Pal Spawn", meta = (ClampMin = "0.1"))
	float EvaluationIntervalSeconds = 2.0f;

private:
	UPROPERTY()
	TArray<TObjectPtr<APW_WildPalSpawner>> RegisteredSpawners;

	FTimerHandle EvaluationTimerHandle;
	int32 ActivePalCount = 0;

	bool CanRunAuthoritySpawn() const;
	void EvaluateSpawners();
	void GatherPlayerLocations(TArray<FVector>& OutPlayerLocations) const;
};
