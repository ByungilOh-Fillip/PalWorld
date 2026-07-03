// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "GameplayTagContainer.h"
#include "Player/Types/PWPlayerGameplayTypes.h"
#include "PWPlayerPrimaryActionComponent.generated.h"

class APWLocalDamageFloatActor;
class APWPlayerCharacter;
class UPWPrimaryActionDataAsset;
class UAnimMontage;

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent, DisplayName = "PW Player Primary Action Component"))
class PALWORLD_API UPWPlayerPrimaryActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerPrimaryActionComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Player|PrimaryAction")
	void TryStartPrimaryAction();

	UFUNCTION(BlueprintCallable, Category = "Player|PrimaryAction")
	void TryStopPrimaryAction();

	UFUNCTION(BlueprintCallable, Category = "Player|PrimaryAction")
	void HandlePrimaryActionHitNotify();

	UFUNCTION(BlueprintCallable, Category = "Player|PrimaryAction")
	void SetToolType(EPWToolType NewToolType);

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction")
	EPWToolType GetCurrentToolType() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|PrimaryAction", meta = (DisplayName = "On Primary Action Started"))
	void BP_OnPrimaryActionStarted(AActor* TargetActor, EPWToolType ToolType);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|PrimaryAction", meta = (DisplayName = "On Primary Action Failed"))
	void BP_OnPrimaryActionFailed();

	// 서버가 확정한 데미지 결과를 소유 클라이언트에서만 표시한다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|PrimaryAction|UI", meta = (DisplayName = "On Local Damage Float"))
	void BP_OnLocalDamageFloat(float AppliedDamage, FVector WorldLocation, EPWToolType ToolType, EPWResourceType ResourceType);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Player|PrimaryAction")
	TObjectPtr<UPWPrimaryActionDataAsset> PrimaryActionData;

	UPROPERTY(EditDefaultsOnly, Category = "Player|PrimaryAction|UI")
	TSubclassOf<APWLocalDamageFloatActor> DamageFloatActorClass;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentToolType, VisibleInstanceOnly, Category = "Player|PrimaryAction")
	EPWToolType CurrentToolType = EPWToolType::Hand;

	FTimerHandle ActionTimerHandle;
	TArray<FTimerHandle> ScheduledHitTimerHandles;

	UFUNCTION(Server, Reliable)
	void ServerRequestPrimaryAction(FVector_NetQuantize ViewLocation, FVector_NetQuantizeNormal ViewDirection);

	UFUNCTION(Server, Reliable)
	void ServerRequestStopPrimaryAction();

	UFUNCTION(Server, Reliable)
	void ServerSetToolType(EPWToolType NewToolType);

	UFUNCTION(Client, Unreliable)
	void ClientShowDamage(float AppliedDamage, FVector_NetQuantize WorldLocation, EPWToolType ToolType, EPWResourceType ResourceType);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayPrimaryActionAnimation(EPWToolType ToolType);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopPrimaryActionAnimation(EPWToolType ToolType);

	UFUNCTION()
	void OnRep_CurrentToolType();

	bool bPrimaryActionHeld = false;
	bool bPrimaryActionFacingLocked = false;
	float LastPrimaryActionHitTime = -FLT_MAX;

	APWPlayerCharacter* GetPlayerCharacter() const;
	const UPWPrimaryActionDataAsset* GetActionData() const;
	float GetTraceDistance() const;
	float GetMaxViewLocationDistance() const;
	float GetStaminaCost() const;
	float GetActionDuration() const;
	float GetDamageVarianceRatio() const;
	float GetMinHarvestHitInterval() const;
	bool GetView(FVector& OutLocation, FVector& OutDirection) const;
	bool FindTargetFromView(FHitResult& OutHitResult) const;
	bool FindTargetFromViewData(const FVector& ViewLocation, const FVector& ViewDirection, FHitResult& OutHitResult) const;
	bool FindTarget(const FVector& TraceStart, const FVector& TraceDirection, FHitResult& OutHitResult) const;
	bool IsViewLocationAllowed(const FVector& ViewLocation) const;
	bool CanDamageTarget(const FHitResult& HitResult) const;
	EPWToolType ResolveCurrentToolType() const;
	EPWResourceType ResolveResourceType(const FHitResult& HitResult) const;
	EPWResourceType ResolveResourceTypeFromReward(FName RewardName, const FGameplayTag& RequiredWorkTag) const;
	float ResolveDamage(EPWResourceType ResourceType) const;
	float ApplyDamageVariance(float BaseDamage) const;
	bool ApplyDamageToTarget(const FHitResult& HitResult, float DamageAmount, float& OutAppliedDamage) const;
	bool ApplyPrimaryActionHitAuthority(const FVector& ViewLocation, const FVector& ViewDirection);
	void ScheduleHarvestHitTimers();
	void ClearScheduledHitTimers();
	void PerformScheduledPrimaryActionHit();
	void StartAuthority(const FHitResult& HitResult, const FVector& ActionDirection);
	void StopAuthority();
	void FinishAction();
	bool ShouldPlayPrimaryActionAnimation(EPWToolType ToolType) const;
	bool IsHarvestHitNotifyName(FName NotifyName) const;
	TArray<float> GetPrimaryActionHitTimes(const UAnimMontage* Montage) const;
	float GetActionRepeatDuration(EPWToolType ToolType) const;
	void PlayPrimaryActionAnimation(EPWToolType ToolType);
	void StopPrimaryActionAnimation(EPWToolType ToolType);
	void ShowDamageLocal(float AppliedDamage, const FVector& WorldLocation, EPWToolType ToolType, EPWResourceType ResourceType);
};
