#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BaseTypes.h"
#include "GameFramework/Actor.h"
#include "PW_BaseCampActor.generated.h"

class UPW_BaseInventoryAggregatorComponent;
class UPW_BaseNavigationComponent;
class UPW_BaseOwnershipComponent;
class UPW_BasePalAssignmentComponent;
class UPW_BaseWorkSimulationComponent;
class UPW_BaseWorkTargetRegistryComponent;
class UNavigationInvokerComponent;
class USceneComponent;

UCLASS(Blueprintable)
class PALWORLD_API APW_BaseCampActor : public AActor
{
	GENERATED_BODY()

public:
	APW_BaseCampActor();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "PW|Base")
	FPW_BaseCampId GetBaseCampId() const { return BaseCampId; }

	UFUNCTION(BlueprintPure, Category = "PW|Base")
	FPW_BaseOwnerId GetBaseOwnerId() const { return OwnerId; }

	UFUNCTION(BlueprintCallable, Category = "PW|Base")
	void SetBaseOwnerId(const FPW_BaseOwnerId& NewOwnerId);

	UFUNCTION(BlueprintPure, Category = "PW|Base")
	float GetCampRadius() const { return CampRadius; }

	UFUNCTION(BlueprintPure, Category = "PW|Base")
	int32 GetCampLevel() const { return CampLevel; }

	UFUNCTION(BlueprintPure, Category = "PW|Base")
	bool ContainsLocation(const FVector& Location) const;

	UFUNCTION(BlueprintPure, Category = "PW|Base")
	UPW_BaseInventoryAggregatorComponent* GetInventoryAggregatorComponent() const { return InventoryAggregatorComponent; }

	UFUNCTION(BlueprintPure, Category = "PW|Base")
	UPW_BaseWorkTargetRegistryComponent* GetWorkTargetRegistryComponent() const { return WorkTargetRegistryComponent; }

	UFUNCTION(BlueprintPure, Category = "PW|Base")
	UPW_BaseNavigationComponent* GetBaseNavigationComponent() const { return BaseNavigationComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Base")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Base")
	TObjectPtr<UPW_BaseOwnershipComponent> OwnershipComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Base")
	TObjectPtr<UPW_BaseInventoryAggregatorComponent> InventoryAggregatorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Base")
	TObjectPtr<UPW_BasePalAssignmentComponent> PalAssignmentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Base")
	TObjectPtr<UPW_BaseWorkTargetRegistryComponent> WorkTargetRegistryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Base")
	TObjectPtr<UPW_BaseWorkSimulationComponent> WorkSimulationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Base")
	TObjectPtr<UPW_BaseNavigationComponent> BaseNavigationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Base|Navigation")
	TObjectPtr<UNavigationInvokerComponent> NavigationInvokerComponent;

	UPROPERTY(Replicated, EditInstanceOnly, BlueprintReadOnly, Category = "PW|Base")
	FPW_BaseCampId BaseCampId;

	UPROPERTY(Replicated, EditInstanceOnly, BlueprintReadOnly, Category = "PW|Base")
	FPW_BaseOwnerId OwnerId;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "PW|Base", meta = (ClampMin = "1.0"))
	float CampRadius = 3000.0f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "PW|Base", meta = (ClampMin = "1"))
	int32 CampLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Debug")
	bool bDrawDebugRadius = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Debug")
	bool bDrawDebugNavigation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Activation", meta = (ClampMin = "0.0"))
	float VisitorActivationExtraRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Activation", meta = (ClampMin = "0.1"))
	float VisitorCheckIntervalSeconds = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Activation", meta = (ClampMin = "0.0"))
	float VisitorKeepAliveSeconds = 90.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Base|Activation")
	bool bHasActiveVisitor = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Base|Activation")
	double LastVisitedServerTime = 0.0;

private:
	FTimerHandle VisitorCheckTimerHandle;

	void RegisterWithSubsystem();
	void UnregisterFromSubsystem();
	void DrawDebugCampRadius() const;
	void StartVisitorChecks();
	void StopVisitorChecks();
	void EvaluateVisitorActivity();
	bool IsAnyPlayerWithinActivationRadius() const;
	void SetBaseActiveState(bool bNewHasActiveVisitor);
};
