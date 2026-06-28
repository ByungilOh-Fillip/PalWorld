// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PWPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UPWPalCommandComponent;
class UPWPlayerActionComponent;
class UPWPlayerCaptureComponent;
class UPWPlayerClimbComponent;
class UPWPlayerCombatComponent;
class UPWPlayerGatherComponent;
class UPWPlayerInteractionComponent;
class UPWPlayerInventoryLinkComponent;
class UPWPlayerMountComponent;
class UPWPlayerSkillComponent;
class UPWPlayerStatComponent;

UCLASS()
class PALWORLD_API APWPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APWPlayerCharacter();

	// 입력 래퍼는 CharacterMovement 또는 전용 컴포넌트로 전달한다.
	void Move(const FVector2D& MovementVector);
	void Look(const FVector2D& LookVector);
	void StartJump();
	void StopJump();
	void StartSprint();
	void StopSprint();
	void StartCrouch();
	void StopCrouch();
	void StartRoll();
	void StartGather();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintPure, Category = "Player|Movement")
	bool IsSprinting() const;

	bool ShouldDrainSprintStamina() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement")
	bool IsRolling() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	bool IsWallClimbing() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	bool IsWallClimbTopOut() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	bool IsClimbing() const { return IsWallClimbing(); }

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	float GetClimbInputX() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	float GetClimbInputY() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	float GetWallClimbVerticalSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	float GetWallClimbHorizontalSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	float GetWallClimbHorizontalBlendValue() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement|Climb")
	float GetWallClimbVerticalBlendValue() const;

	UPWPlayerActionComponent* GetActionComponent() const { return ActionComponent; }
	UPWPlayerStatComponent* GetStatComponent() const { return StatComponent; }
	UPWPlayerGatherComponent* GetGatherComponent() const { return GatherComponent; }
	UPWPlayerClimbComponent* GetClimbComponent() const { return ClimbComponent; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Player|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Player|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = "Player|Components")
	TObjectPtr<UPWPlayerActionComponent> ActionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Player|Components")
	TObjectPtr<UPWPlayerStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, Category = "Player|Components")
	TObjectPtr<UPWPlayerCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, Category = "Player|Components")
	TObjectPtr<UPWPlayerGatherComponent> GatherComponent;

	UPROPERTY(VisibleAnywhere, Category = "Player|Components")
	TObjectPtr<UPWPlayerSkillComponent> SkillComponent;

	UPROPERTY(VisibleAnywhere, Category = "Player|Components")
	TObjectPtr<UPWPalCommandComponent> PalCommandComponent;

	UPROPERTY(VisibleAnywhere, Category = "Player|Components")
	TObjectPtr<UPWPlayerInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Player|Components")
	TObjectPtr<UPWPlayerInventoryLinkComponent> InventoryLinkComponent;

	UPROPERTY(VisibleAnywhere, Category = "Player|Components")
	TObjectPtr<UPWPlayerCaptureComponent> CaptureComponent;

	UPROPERTY(VisibleAnywhere, Category = "Player|Components")
	TObjectPtr<UPWPlayerMountComponent> MountComponent;

	UPROPERTY(VisibleAnywhere, Category = "Player|Components")
	TObjectPtr<UPWPlayerClimbComponent> ClimbComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement")
	float WalkSpeed = 480.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement")
	float SprintSpeed = 850.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement")
	float CrouchedWalkSpeed = 220.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement", meta = (ClampMin = "0.0"))
	float MinSprintActiveSpeed = 10.f;

	UPROPERTY(ReplicatedUsing = OnRep_IsSprinting)
	bool bIsSprinting = false;

	// 달리기는 커스텀 속도 상태라 CharacterMovement 기본 이동과 별도로 복제한다.
	UFUNCTION()
	void OnRep_IsSprinting();

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewIsSprinting);

	bool CanStartSprint() const;
	void SetSprinting(bool bNewIsSprinting);
	bool IsSprintMovementActive() const;
	void ApplyMovementSpeed();
};
