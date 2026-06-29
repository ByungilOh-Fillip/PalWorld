// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/Types/PWPlayerGameplayTypes.h"
#include "PWGatherableResourceActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class PALWORLD_API APWGatherableResourceActor : public AActor
{
	GENERATED_BODY()

public:
	APWGatherableResourceActor();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Player|Gather")
	EPWResourceType GetResourceType() const { return ResourceType; }

	UFUNCTION(BlueprintPure, Category = "Player|Gather")
	EPWResourceDropType GetDropType() const { return DropType; }

	UFUNCTION(BlueprintPure, Category = "Player|Gather")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Player|Gather")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Player|Gather")
	bool IsDepleted() const { return CurrentHealth <= 0.f; }

	bool ApplyGatherDamage(AActor* GatherInstigator, float DamageAmount, EPWToolType ToolType, float& OutAppliedDamage);

protected:
	// 서버에서 자원 데미지를 받을 때 BP 연출을 붙일 수 있는 지점.
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Gather", meta = (DisplayName = "On Gather Damaged"))
	void BP_OnGatherDamaged(AActor* GatherInstigator, float AppliedDamage, EPWToolType ToolType, float NewHealth);

	// 누적 데미지가 DropDamageInterval을 넘을 때마다 호출된다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Gather", meta = (DisplayName = "On Gather Drop Generated"))
	void BP_OnGatherDropGenerated(AActor* GatherInstigator, EPWResourceDropType GeneratedDropType, int32 DropCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Gather", meta = (DisplayName = "On Resource Depleted"))
	void BP_OnResourceDepleted(AActor* GatherInstigator);

private:
	UPROPERTY(VisibleAnywhere, Category = "Player|Gather|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Player|Gather|Components")
	TObjectPtr<UStaticMeshComponent> ResourceMesh;

	UPROPERTY(EditAnywhere, Category = "Player|Gather")
	EPWResourceType ResourceType = EPWResourceType::Tree;

	UPROPERTY(EditAnywhere, Category = "Player|Gather")
	EPWResourceDropType DropType = EPWResourceDropType::Wood;

	UPROPERTY(EditAnywhere, Category = "Player|Gather", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleInstanceOnly, Category = "Player|Gather")
	float CurrentHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Player|Gather", meta = (ClampMin = "1.0"))
	float DropDamageInterval = 25.f;

	UPROPERTY(EditAnywhere, Category = "Player|Gather", meta = (ClampMin = "1"))
	int32 DropCountPerInterval = 1;

	UPROPERTY(EditAnywhere, Category = "Player|Gather")
	bool bDestroyWhenDepleted = true;

	float DamageSinceLastDrop = 0.f;

	UFUNCTION()
	void OnRep_CurrentHealth();

	void GenerateDrops(AActor* GatherInstigator, int32 IntervalCount);
	void DepleteResource(AActor* GatherInstigator);
};
