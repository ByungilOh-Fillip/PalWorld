// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "Player/Types/PWPlayerGameplayTypes.h"
#include "PWPlayerGatherComponent.generated.h"

class APWGatherableResourceActor;
class APWPlayerCharacter;
class APWLocalDamageFloatActor;

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
	EPWToolType GetCurrentToolType() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Gather", meta = (DisplayName = "On Gather Started"))
	void BP_OnGatherStarted(APWGatherableResourceActor* TargetResource, EPWToolType ToolType);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Gather", meta = (DisplayName = "On Gather Failed"))
	void BP_OnGatherFailed();

	// 서버가 확정한 데미지 결과를 소유 클라이언트에서만 표시한다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Gather|UI", meta = (DisplayName = "On Local Gather Damage Float"))
	void BP_OnLocalGatherDamageFloat(float AppliedDamage, FVector WorldLocation, EPWToolType ToolType, EPWResourceType ResourceType);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather", meta = (ClampMin = "50.0"))
	float GatherTraceDistance = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather", meta = (ClampMin = "100.0"))
	float MaxGatherViewLocationDistance = 800.f;

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

	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather|Damage", meta = (ClampMin = "0.0", ClampMax = "0.25"))
	float DamageVarianceRatio = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Gather|UI")
	TSubclassOf<APWLocalDamageFloatActor> DamageFloatActorClass;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentToolType, VisibleInstanceOnly, Category = "Player|Gather")
	EPWToolType CurrentToolType = EPWToolType::Hand;

	FTimerHandle GatherActionTimerHandle;

	UFUNCTION(Server, Reliable)
	void ServerRequestGatherFromView(FVector_NetQuantize RequestedViewLocation, FVector_NetQuantizeNormal RequestedViewDirection);

	UFUNCTION(Server, Reliable)
	void ServerRequestEquipTool(EPWToolType NewToolType);

	UFUNCTION(Client, Unreliable)
	void ClientShowGatherDamage(float AppliedDamage, FVector_NetQuantize WorldLocation, EPWToolType ToolType, EPWResourceType ResourceType);

	UFUNCTION()
	void OnRep_CurrentToolType();

	APWPlayerCharacter* GetPlayerCharacter() const;
	bool GetGatherView(FVector& OutViewLocation, FVector& OutViewDirection) const;
	APWGatherableResourceActor* FindGatherTargetFromView() const;
	APWGatherableResourceActor* FindGatherTargetFromViewData(const FVector& ViewLocation, const FVector& ViewDirection) const;
	APWGatherableResourceActor* FindGatherTarget(const FVector& TraceStart, const FVector& TraceDirection) const;
	bool IsGatherViewLocationAllowed(const FVector& ViewLocation) const;
	bool CanGatherTarget(const APWGatherableResourceActor* TargetResource) const;
	EPWToolType ResolveCurrentToolType() const;
	float ResolveGatherDamage(EPWResourceType ResourceType) const;
	float ApplyDamageVariance(float BaseDamage) const;
	void GatherAuthority(APWGatherableResourceActor* TargetResource, const FVector& ActionDirection);
	void FinishGatherAction();
	void ShowGatherDamageLocal(float AppliedDamage, const FVector& WorldLocation, EPWToolType ToolType, EPWResourceType ResourceType);
};
