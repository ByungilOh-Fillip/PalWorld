#pragma once

#include "CoreMinimal.h"
#include "Map/PW_MapTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "PW_MapSubsystem.generated.h"

class APawn;

struct FPW_MapExplorationRuntimeState
{
	TSet<int32> VisitedCellIndices;
};

UCLASS()
class PALWORLD_API UPW_MapSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "PW|Map")
	void SetMapBounds(const FVector2D& NewWorldMin, const FVector2D& NewWorldMax);

	UFUNCTION(BlueprintCallable, Category = "PW|Map")
	void SetExplorationGridSize(int32 NewGridWidth, int32 NewGridHeight);

	UFUNCTION(BlueprintCallable, Category = "PW|Map")
	void SetRevealRadius(float NewRevealRadius);

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	FName GetMapId() const { return MapId; }

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	FVector2D WorldLocationToMapUV(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Map")
	void RevealAroundLocation(const FString& PlayerId, const FVector& WorldLocation);

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	void GetVisitedCellIndices(const FString& PlayerId, TArray<int32>& OutVisitedCellIndices) const;

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	void GetMapMarkers(TArray<FPW_MapMarker>& OutMarkers) const;

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	bool GetLocalPlayerMapUV(FVector2D& OutPlayerMapUV) const;

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	float GetRevealRadiusUV() const;

	UFUNCTION(BlueprintPure, Category = "PW|Map|Save")
	FPW_MapExplorationSaveData MakeExplorationSaveData(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Map|Save")
	bool ApplyExplorationSaveData(const FPW_MapExplorationSaveData& SaveData);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	FName MapId = TEXT("Default");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	FVector2D WorldMin = FVector2D(-50000.0f, -50000.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	FVector2D WorldMax = FVector2D(50000.0f, 50000.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map", meta = (ClampMin = "1"))
	int32 GridWidth = 128;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map", meta = (ClampMin = "1"))
	int32 GridHeight = 128;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map", meta = (ClampMin = "0.0"))
	float RevealRadius = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map", meta = (ClampMin = "0.05"))
	float ExplorationUpdateIntervalSeconds = 0.5f;

private:
	FTimerHandle ExplorationUpdateTimerHandle;
	TMap<FString, FPW_MapExplorationRuntimeState> ExplorationByPlayerId;

	bool IsMapConfigured() const;
	bool IsValidCellIndex(int32 CellIndex) const;
	int32 GetCellIndex(int32 CellX, int32 CellY) const;
	FString GetLocalPlayerId() const;
	APawn* GetLocalPlayerPawn() const;
	void UpdateLocalPlayerExploration();
	void AddVisitedCell(const FString& PlayerId, int32 CellX, int32 CellY);
	void AddTeleportMarkers(TArray<FPW_MapMarker>& OutMarkers) const;
	void AddBaseCampMarkers(TArray<FPW_MapMarker>& OutMarkers) const;
};
