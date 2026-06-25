// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "TimerManager.h"
#include "PWPlayerActionComponent.generated.h"

class APWPlayerCharacter;
class UAnimMontage;

UENUM(BlueprintType)
enum class EPWPlayerActionState : uint8
{
	None,
	Rolling,
	Interacting,
	Gathering,
	Attacking,
	UsingSkill
};

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerActionComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void TryStartRoll();

	UFUNCTION(BlueprintPure, Category = "Player|Action")
	EPWPlayerActionState GetCurrentActionState() const { return CurrentActionState; }

	UFUNCTION(BlueprintPure, Category = "Player|Action")
	bool IsBusy() const { return CurrentActionState != EPWPlayerActionState::None; }

	UFUNCTION(BlueprintPure, Category = "Player|Action")
	bool IsRolling() const { return CurrentActionState == EPWPlayerActionState::Rolling; }

	void SetRollMontage(UAnimMontage* InRollMontage);
	void SetRollTuning(float InRollDistance, float InRollMovementDuration, bool bInUseCodeDrivenMovement);

private:
	// 구르기 데이터는 컴포넌트 기본값/BP에 넣어 서버와 클라가 같은 몽타주를 참조하게 한다.
	UPROPERTY(EditDefaultsOnly, Category = "Player|Action|Roll")
	TObjectPtr<UAnimMontage> RollMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Action|Roll", meta = (ClampMin = "0.1"))
	float RollFallbackDuration = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Action|Roll", meta = (ClampMin = "0.0"))
	float RollDistance = 550.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Action|Roll", meta = (ClampMin = "0.05"))
	float RollMovementDuration = 0.42f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Action|Roll")
	bool bUseCodeDrivenRollMovement = false;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Action|Roll", meta = (ClampMin = "1"))
	int32 RollRootMotionPriority = 500;

	// 배타 행동 상태의 원본은 서버다. 클라는 이 값으로 애니메이션/입력 차단을 따라간다.
	UPROPERTY(ReplicatedUsing = OnRep_CurrentActionState)
	EPWPlayerActionState CurrentActionState = EPWPlayerActionState::None;

	FTimerHandle RollTimerHandle;
	uint16 RollRootMotionSourceId = 0;

	UFUNCTION()
	void OnRep_CurrentActionState();

	// 네트워크 진입점.
	UFUNCTION(Server, Reliable)
	void ServerRequestStartRoll(FVector_NetQuantizeNormal RequestedDirection);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartRollVisuals(FRotator RollFacingRotation);

	// 공통 검증/헬퍼.
	APWPlayerCharacter* GetPlayerCharacter() const;
	bool CanStartAction(EPWPlayerActionState RequestedActionState) const;
	bool CanStartRoll() const;
	float GetRollDuration() const;
	FVector GetRollDirection() const;
	FVector ResolveRollDirection(const FVector& RequestedDirection) const;

	// 서버 권한 구르기 흐름.
	void SetActionStateAuthority(EPWPlayerActionState NewActionState);
	void StartRollAuthority(const FVector& RequestedDirection);
	void FinishRoll();

	// 연출/이동 헬퍼.
	void PlayRollMontage();
	void ApplyRollMovement(const FVector& RollDirection, float RollDuration);
	void ClearRollMovement();
};
