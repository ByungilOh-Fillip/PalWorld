#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PWPalBase.h"
#include "PW_WildPalSpawner.generated.h"

class UPW_WildPalSpawnSubsystem;

UENUM(BlueprintType)
enum class EPW_WildPalSpawnTimeRule : uint8
{
	Always,
	DayOnly,
	NightOnly
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_WildPalSpawnEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn")
	TSubclassOf<APWPalBase> PalClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn")
	EPW_WildPalSpawnTimeRule TimeRule = EPW_WildPalSpawnTimeRule::Always;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPW_OnWildPalSpawned, APWPalBase*, SpawnedPal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPW_OnWildPalDespawned, APWPalBase*, DespawnedPal);

UCLASS(Blueprintable)
class PALWORLD_API APW_WildPalSpawner : public AActor
{
	GENERATED_BODY()

public:
	APW_WildPalSpawner();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void EvaluateSpawner(UPW_WildPalSpawnSubsystem& SpawnSubsystem);

	UPROPERTY(BlueprintAssignable, Category = "PW|Wild Pal Spawn")
	FPW_OnWildPalSpawned OnWildPalSpawned;

	UPROPERTY(BlueprintAssignable, Category = "PW|Wild Pal Spawn")
	FPW_OnWildPalDespawned OnWildPalDespawned;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn")
	TArray<FPW_WildPalSpawnEntry> SpawnEntries;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn", meta = (ClampMin = "0"))
	int32 MaxActivePals = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn", meta = (ClampMin = "0.0"))
	float SpawnRadius = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn", meta = (ClampMin = "0.1"))
	float RespawnIntervalSeconds = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn", meta = (ClampMin = "0.0"))
	float ActivationDistance = 8000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn", meta = (ClampMin = "0.0"))
	float DespawnDistance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn", meta = (ClampMin = "0.0"))
	float MinSpawnDistanceFromPlayer = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn", meta = (ClampMin = "1"))
	int32 MaxSpawnLocationAttempts = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn", meta = (ClampMin = "0.0"))
	float SpawnNetCullDistance = 12000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Wild Pal Spawn|Debug")
	bool bDrawDebugSpawnRadius = true;

private:
	UPROPERTY()
	TArray<TObjectPtr<APWPalBase>> ActiveSpawnedPals;

	double LastSpawnTimeSeconds = -TNumericLimits<double>::Max();

	bool CanRunAuthoritySpawner() const;
	void RegisterWithSubsystem();
	void UnregisterFromSubsystem();
	void CleanupInvalidActivePals(UPW_WildPalSpawnSubsystem& SpawnSubsystem);
	void ReturnAllSpawnedPals(UPW_WildPalSpawnSubsystem& SpawnSubsystem);
	bool TrySpawnOnePal(UPW_WildPalSpawnSubsystem& SpawnSubsystem);
	const FPW_WildPalSpawnEntry* ChooseSpawnEntry() const;
	bool DoesEntryMatchWorldTime(const FPW_WildPalSpawnEntry& Entry) const;
	bool FindSpawnTransform(UPW_WildPalSpawnSubsystem& SpawnSubsystem, FTransform& OutSpawnTransform) const;
	void DrawDebugSpawnRadius() const;

	UFUNCTION()
	void HandleSpawnedPalDestroyed(AActor* DestroyedActor);
};
