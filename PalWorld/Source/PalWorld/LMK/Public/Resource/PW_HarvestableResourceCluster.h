#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/PW_HarvestInstanceDamageTarget.h"
#include "PW_HarvestableResourceCluster.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UPW_HarvestableResourceClusterComponent;

UCLASS()
class PALWORLD_API APW_HarvestableResourceCluster : public AActor, public IPW_HarvestInstanceDamageTarget
{
	GENERATED_BODY()

public:
	APW_HarvestableResourceCluster();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool ApplyHarvestDamageToInstance_Implementation(int32 InstanceIndex, float DamageAmount, AActor* InstigatorActor) override;

	UFUNCTION(BlueprintPure, Category = "PW Harvest Cluster")
	UPW_HarvestableResourceClusterComponent* GetClusterResourceComponent() const { return ClusterComponent; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW Harvest Cluster Components")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> HarvestInstances;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW Harvest Cluster Components")
	TObjectPtr<UPW_HarvestableResourceClusterComponent> ClusterComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW Harvest Cluster Placement")
	TArray<FTransform> InstanceTransforms;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW Harvest Cluster Depletion")
	TSubclassOf<AActor> DepletionActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW Harvest Cluster Depletion", meta = (ClampMin = "0.0"))
	float DepletionActorLifeTime = 5.0f;

private:
	UFUNCTION()
	void HandleInstanceDepleted(int32 InstanceIndex, AActor* InstigatorActor);

	UFUNCTION()
	void HandleInstanceRespawned(int32 InstanceIndex);

	UFUNCTION()
	void HandleReplicatedStateChanged();

	void RebuildInstances();
	void ApplyReplicatedInstanceStates();
	void SetInstanceHidden(int32 InstanceIndex, bool bShouldHide);
	void SpawnDepletionActor(int32 InstanceIndex, AActor* InstigatorActor);
	bool GetInstanceWorldTransform(int32 InstanceIndex, FTransform& OutTransform) const;
};
