// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "Player/Types/PWPlayerGameplayTypes.h"
#include "PWPlayerGatherComponent.generated.h"

class APWGatherableResourceActor;
class APWPlayerCharacter;

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerGatherComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerGatherComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Player|Gather")
	void TryGatherFromView();

	UFUNCTION(BlueprintCallable, Category = "Player|Gather")
	void RequestEquipTool(EPWToolType NewToolType);

	UFUNCTION(BlueprintPure, Category = "Player|Gather")
	EPWToolType GetCurrentToolType() const { return CurrentToolType; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Gather", meta = (DisplayName = "On Gather Started"))
	void BP_OnGatherStarted(APWGatherableResourceActor* TargetResource, EPWToolType ToolType);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Gather", meta = (DisplayName = "On Gather Failed"))
	void BP_OnGatherFailed();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather", meta = (ClampMin = "50.0"))
	float GatherTraceDistance = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather", meta = (ClampMin = "0.0"))
	float GatherStaminaCost = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather", meta = (ClampMin = "0.05"))
	float GatherActionDuration = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather|Damage", meta = (ClampMin = "0.0"))
	float HandTreeDamage = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather|Damage", meta = (ClampMin = "0.0"))
	float AxeTreeDamage = 25.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather|Damage", meta = (ClampMin = "0.0"))
	float PickaxeTreeDamage = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather|Damage", meta = (ClampMin = "0.0"))
	float HandStoneDamage = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather|Damage", meta = (ClampMin = "0.0"))
	float AxeStoneDamage = 6.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather|Damage", meta = (ClampMin = "0.0"))
	float PickaxeStoneDamage = 25.f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentToolType, VisibleInstanceOnly, Category = "Player|Gather")
	EPWToolType CurrentToolType = EPWToolType::Hand;

	FTimerHandle GatherActionTimerHandle;

	UFUNCTION(Server, Reliable)
	void ServerRequestGather(APWGatherableResourceActor* TargetResource);

	UFUNCTION(Server, Reliable)
	void ServerRequestEquipTool(EPWToolType NewToolType);

	UFUNCTION()
	void OnRep_CurrentToolType();

	APWPlayerCharacter* GetPlayerCharacter() const;
	APWGatherableResourceActor* FindGatherTargetFromView() const;
	bool CanGatherTarget(const APWGatherableResourceActor* TargetResource) const;
	float ResolveGatherDamage(EPWResourceType ResourceType) const;
	void GatherAuthority(APWGatherableResourceActor* TargetResource);
	void FinishGatherAction();
};
