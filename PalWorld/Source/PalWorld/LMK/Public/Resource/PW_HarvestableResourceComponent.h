#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PW_HarvestableResourceComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FPWHarvestedSignature,
	AActor*, InstigatorActor,
	float, DamageAmount,
	float, CurrentHealth,
	FName, RewardName,
	int32, RewardAmount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPWHarvestDepletedSignature, AActor*, InstigatorActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPWHarvestRespawnedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPWHarvestDepletedStateChangedSignature, bool, bNewIsDepleted);

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_HarvestableResourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_HarvestableResourceComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "PW|Harvest")
	bool ApplyHarvestDamage(float DamageAmount, AActor* InstigatorActor);

	UFUNCTION(BlueprintPure, Category = "PW|Harvest")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "PW|Harvest")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "PW|Harvest")
	bool IsDepleted() const { return bIsDepleted; }

	UFUNCTION(BlueprintPure, Category = "PW|Harvest")
	FGameplayTag GetRequiredWorkTag() const { return RequiredWorkTag; }

	UFUNCTION(BlueprintPure, Category = "PW|Harvest")
	FName GetRewardName() const { return RewardName; }

	UFUNCTION(BlueprintPure, Category = "PW|Harvest")
	int32 GetRewardAmount() const { return RewardAmount; }

	UFUNCTION(BlueprintPure, Category = "PW|Harvest")
	float GetRewardDamageInterval() const { return RewardDamageInterval; }

	void SetResourceDefaults(FGameplayTag InRequiredWorkTag, FName InRewardName, int32 InRewardAmount, float InRewardDamageInterval = 25.f);

	UPROPERTY(BlueprintAssignable, Category = "PW|Harvest")
	FPWHarvestedSignature OnHarvested;

	UPROPERTY(BlueprintAssignable, Category = "PW|Harvest")
	FPWHarvestDepletedSignature OnDepleted;

	UPROPERTY(BlueprintAssignable, Category = "PW|Harvest")
	FPWHarvestRespawnedSignature OnRespawned;

	UPROPERTY(BlueprintAssignable, Category = "PW|Harvest")
	FPWHarvestDepletedStateChangedSignature OnDepletedStateChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Harvest")
	float CurrentHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_IsDepleted, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Harvest")
	bool bIsDepleted = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest", meta = (ClampMin = "0.0"))
	float RespawnDelay = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest")
	FGameplayTag RequiredWorkTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest")
	FName RewardName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest", meta = (ClampMin = "0"))
	int32 RewardAmount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest", meta = (ClampMin = "0.0"))
	float RewardDamageInterval = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest|Auto Collect", meta = (ClampMin = "0.0"))
	float AutoCollectMinDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest|Auto Collect", meta = (ClampMin = "0.0"))
	float AutoCollectMaxDelay = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest|Auto Collect", meta = (ClampMin = "0.0"))
	float AutoCollectRadius = 900.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest|Drop")
	bool bSpawnWorldDrop = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest|Drop", meta = (EditCondition = "bSpawnWorldDrop"))
	TSubclassOf<class APWWorldItemActor> WorldItemActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest|Drop", meta = (ClampMin = "0.0", EditCondition = "bSpawnWorldDrop"))
	float DropScatterRadius = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest|Drop", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bSpawnWorldDrop"))
	float DropTowardInstigatorMinAlpha = 0.55f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest|Drop", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bSpawnWorldDrop"))
	float DropTowardInstigatorMaxAlpha = 0.85f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Harvest|Drop", meta = (EditCondition = "bSpawnWorldDrop"))
	FVector DropLocationOffset = FVector(0.f, 0.f, 30.f);

private:
	FTimerHandle RespawnTimerHandle;

	UPROPERTY(Transient)
	float RewardDamageProgress = 0.f;

	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
	void OnRep_IsDepleted();

	void DepleteResource(AActor* InstigatorActor);
	void RespawnResource();
	void BroadcastDepletedState();
	int32 ConsumeRewardIntervals(float AppliedDamage);
	AActor* ResolveRewardReceiver(AActor* InstigatorActor) const;
	class UPWItemDataAsset* ResolveRewardItemData(AActor* RewardReceiver, FName GrantedRewardName) const;
	void GrantRewardDirectDelayed(AActor* RewardReceiver, FName GrantedRewardName, int32 GrantedRewardAmount);
	void GrantReward(AActor* InstigatorActor, int32 RewardMultiplier);
};
