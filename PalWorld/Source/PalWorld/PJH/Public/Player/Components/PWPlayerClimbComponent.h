// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "PWPlayerClimbComponent.generated.h"

class APWPlayerCharacter;
class UCharacterMovementComponent;

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerClimbComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerClimbComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool TryStartClimb();
	void StopClimb(bool bLaunchOff);
	void SetClimbInput(const FVector2D& MovementVector);

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	bool IsClimbing() const { return bIsClimbing; }

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	bool IsClimbTopOut() const { return bIsClimbTopOut; }

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	float GetClimbInputX() const { return ClimbInputX; }

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	float GetClimbInputY() const { return ClimbInputY; }

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	float GetWallClimbVerticalSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	float GetWallClimbHorizontalSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	float GetWallClimbHorizontalBlendValue() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	float GetWallClimbVerticalBlendValue() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<APWPlayerCharacter> CachedPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb", meta = (ClampMin = "0.0"))
	float ClimbTraceDistance = 95.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb", meta = (ClampMin = "0.0"))
	float ClimbTraceRadius = 24.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb")
	bool bRequireClimbableSurfaceTag = true;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb")
	FName ClimbableSurfaceTag = TEXT("Climbable");

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb", meta = (ClampMin = "0.0"))
	float ClimbDesiredDistance = 48.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb", meta = (ClampMin = "0.0"))
	float ClimbDistanceCorrectionSpeed = 720.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb", meta = (ClampMin = "0.0"))
	float ClimbSpeed = 180.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb", meta = (ClampMin = "0.0"))
	float ClimbStaminaDrainPerSecond = 16.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb", meta = (ClampMin = "0.0"))
	float MinClimbStaminaDrainSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb", meta = (ClampMin = "0.0"))
	float MinClimbStartStamina = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb", meta = (ClampMin = "0.0"))
	float ClimbJumpUpStrength = 430.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb", meta = (ClampMin = "0.0"))
	float ClimbJumpAwayStrength = 360.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb", meta = (ClampMin = "45.0", ClampMax = "90.0"))
	float MinClimbSurfaceAngleDegrees = 70.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb", meta = (ClampMin = "0.0"))
	float ClimbRequestCooldown = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb|Mantle", meta = (ClampMin = "0.0"))
	float ClimbLedgeUpProbe = 110.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb|Mantle", meta = (ClampMin = "0.0"))
	float ClimbLedgeForwardProbe = 70.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb|Mantle", meta = (ClampMin = "0.0"))
	float ClimbLedgeDownProbe = 170.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement|Climb|Mantle", meta = (ClampMin = "0.0"))
	float ClimbLedgeSnapOffset = 4.f;

	UPROPERTY(ReplicatedUsing = OnRep_IsClimbing)
	bool bIsClimbing = false;

	// 아직은 예약 상태다. 추후 Motion Warping/Root Motion TopOut을 붙일 때 true 흐름을 추가한다.
	UPROPERTY(Replicated)
	bool bIsClimbTopOut = false;

	UPROPERTY(ReplicatedUsing = OnRep_WallNormal)
	FVector_NetQuantizeNormal WallNormal = FVector::ZeroVector;

	UPROPERTY(Replicated)
	float ClimbInputX = 0.f;

	UPROPERTY(Replicated)
	float ClimbInputY = 0.f;

	float CachedMaxFlySpeed = 0.f;
	float CachedBrakingDecelerationFlying = 0.f;
	bool bCachedOrientRotationToMovement = true;
	bool bHasCachedMovementValues = false;
	float LastClimbRequestTime = -BIG_NUMBER;

	UFUNCTION()
	void OnRep_IsClimbing();

	UFUNCTION()
	void OnRep_WallNormal();

	UFUNCTION(Server, Reliable)
	void ServerTryStartClimb();

	UFUNCTION(Server, Reliable)
	void ServerStopClimb(bool bLaunchOff);

	UFUNCTION(Server, Reliable)
	void ServerSetClimbInput(float NewClimbInputX, float NewClimbInputY);

	APWPlayerCharacter* GetPlayerCharacter() const;
	UCharacterMovementComponent* GetMovementComponent() const;

	bool CanStartClimb() const;
	bool HasClimbMoveInput() const;
	bool ShouldDrainClimbStamina() const;
	bool FindClimbableWall(FHitResult& OutHit) const;
	bool IsClimbableSurface(const FHitResult& WallHit) const;
	bool IsClimbTouchingGround() const;
	bool TryMantleFromClimb();

	void StartClimbServer(const FVector& InWallNormal);
	void StopClimbInternal(bool bLaunchOff);
	void CacheMovementValues(UCharacterMovementComponent& MovementComponent);
	void RestoreMovementValues(UCharacterMovementComponent& MovementComponent);
	void ApplyClimbMovementMode();
	void ApplyClimbFacing();
	void MoveAlongWall();
	void UpdateClimb(float DeltaSeconds);
	void MaintainClimbDistance(const FHitResult& WallHit, float DeltaSeconds);
	void SetReplicatedClimbInput(float NewClimbInputX, float NewClimbInputY);
	FVector GetClimbRightVector() const;
};
