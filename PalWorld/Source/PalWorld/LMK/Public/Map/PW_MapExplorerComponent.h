#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Map/PW_MapTypes.h"
#include "PW_MapExplorerComponent.generated.h"

class UPW_MapSubsystem;
class AActor;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_MapExplorerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_MapExplorerComponent();

	UFUNCTION(BlueprintCallable, Category = "PW|Map")
	void ConfigureMapSubsystem();

	UFUNCTION(BlueprintCallable, Category = "PW|Map")
	void RevealOwnerLocation();

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	bool GetOwnerMapUV(FVector2D& OutMapUV) const;

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	void GetVisitedCellIndices(TArray<int32>& OutVisitedCellIndices) const;

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	FString GetResolvedPlayerId() const;

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	FVector2D GetWorldMin() const { return WorldMin; }

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	FVector2D GetWorldMax() const { return WorldMax; }

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	int32 GetGridWidth() const { return GridWidth; }

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	int32 GetGridHeight() const { return GridHeight; }

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	float GetRevealRadius() const { return RevealRadius; }

	UFUNCTION(BlueprintPure, Category = "PW|Map|Save")
	FPW_MapExplorationSaveData MakeExplorationSaveData() const;

	UFUNCTION(BlueprintCallable, Category = "PW|Map|Save")
	bool ApplyExplorationSaveData(const FPW_MapExplorationSaveData& SaveData);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	FString PlayerId = TEXT("LocalPlayer");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	bool bUseOwnerNameAsPlayerId = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	bool bApplyMapSettingsOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	bool bAutoReveal = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map", meta = (ClampMin = "0.05"))
	float RevealUpdateIntervalSeconds = 0.5f;

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

private:
	FTimerHandle RevealTimerHandle;

	UPW_MapSubsystem* GetMapSubsystem() const;
	AActor* GetExplorationActor() const;
};
