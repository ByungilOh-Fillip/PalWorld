#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PW_HarvestableResourceClusterComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FPWHarvestInstanceDamagedSignature,
	int32, InstanceIndex,
	AActor*, InstigatorActor,
	float, DamageAmount,
	FName, RewardName,
	int32, RewardAmount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPWHarvestInstanceDepletedSignature, int32, InstanceIndex, AActor*, InstigatorActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPWHarvestInstanceRespawnedSignature, int32, InstanceIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPWHarvestClusterStateChangedSignature);

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_HarvestableResourceClusterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_HarvestableResourceClusterComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeInstances(int32 InstanceCount);
	bool ApplyHarvestDamageToInstance(int32 InstanceIndex, float DamageAmount, AActor* InstigatorActor);

	UFUNCTION(BlueprintPure, Category = "PW Harvest Cluster State")
	bool IsInstanceDepleted(int32 InstanceIndex) const;

	UFUNCTION(BlueprintPure, Category = "PW Harvest Cluster State")
	TArray<int32> GetDepletedInstanceIndices() const { return DepletedInstanceIndices; }

	UFUNCTION(BlueprintPure, Category = "PW Harvest Cluster State")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "PW Harvest Cluster State")
	float GetRespawnDelay() const { return RespawnDelay; }

	UFUNCTION(BlueprintPure, Category = "PW Harvest Cluster State")
	FGameplayTag GetRequiredWorkTag() const { return RequiredWorkTag; }

	UPROPERTY(BlueprintAssignable, Category = "PW Harvest Cluster Events")
	FPWHarvestInstanceDamagedSignature OnInstanceDamaged;

	UPROPERTY(BlueprintAssignable, Category = "PW Harvest Cluster Events")
	FPWHarvestInstanceDepletedSignature OnInstanceDepleted;

	UPROPERTY(BlueprintAssignable, Category = "PW Harvest Cluster Events")
	FPWHarvestInstanceRespawnedSignature OnInstanceRespawned;

	UPROPERTY(BlueprintAssignable, Category = "PW Harvest Cluster Events")
	FPWHarvestClusterStateChangedSignature OnReplicatedStateChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster State", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster State", meta = (ClampMin = "0.0"))
	float RespawnDelay = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster State")
	FGameplayTag RequiredWorkTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster Reward")
	FName RewardName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster Reward", meta = (ClampMin = "0"))
	int32 RewardAmount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster Reward", meta = (ClampMin = "0.0"))
	float RewardDamageInterval = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster Reward|Auto Collect", meta = (ClampMin = "0.0"))
	float AutoCollectMinDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster Reward|Auto Collect", meta = (ClampMin = "0.0"))
	float AutoCollectMaxDelay = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster Reward|Auto Collect", meta = (ClampMin = "0.0"))
	float AutoCollectRadius = 900.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster Reward|Drop")
	bool bSpawnWorldDrop = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster Reward|Drop", meta = (EditCondition = "bSpawnWorldDrop"))
	TSubclassOf<class APWWorldItemActor> WorldItemActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster Reward|Drop", meta = (ClampMin = "0.0", EditCondition = "bSpawnWorldDrop"))
	float DropScatterRadius = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster Reward|Drop", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bSpawnWorldDrop"))
	float DropTowardInstigatorMinAlpha = 0.55f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster Reward|Drop", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bSpawnWorldDrop"))
	float DropTowardInstigatorMaxAlpha = 0.85f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW Harvest Cluster Reward|Drop", meta = (EditCondition = "bSpawnWorldDrop"))
	FVector DropLocationOffset = FVector(0.f, 0.f, 30.f);

	UPROPERTY(ReplicatedUsing = OnRep_DepletedInstanceIndices, Transient)
	TArray<int32> DepletedInstanceIndices;

private:
	UPROPERTY(Transient)
	TArray<float> CurrentHealthByInstance;

	UPROPERTY(Transient)
	TArray<float> RewardDamageProgressByInstance;

	TMap<int32, FTimerHandle> RespawnTimerHandles;

	UFUNCTION()
	void OnRep_DepletedInstanceIndices();

	void DepleteInstance(int32 InstanceIndex, AActor* InstigatorActor);
	void RespawnInstance(int32 InstanceIndex);
	bool IsValidInstanceIndex(int32 InstanceIndex) const;
	FName ResolveRewardName() const;
	int32 ConsumeRewardIntervals(int32 InstanceIndex, float AppliedDamage);
	AActor* ResolveRewardReceiver(AActor* InstigatorActor) const;
	class UPWItemDataAsset* ResolveRewardItemData(AActor* RewardReceiver, FName GrantedRewardName) const;
	FVector GetRewardDropLocation(int32 InstanceIndex) const;
	void GrantRewardDirectDelayed(AActor* RewardReceiver, FName GrantedRewardName, int32 GrantedRewardAmount);
	void GrantReward(AActor* InstigatorActor, int32 RewardMultiplier, int32 InstanceIndex);
};
