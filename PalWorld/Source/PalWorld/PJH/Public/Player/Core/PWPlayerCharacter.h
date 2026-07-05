// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/Character.h"
#include "Interfaces/PW_ItemReceiver.h"
#include "PWPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UPWPalCommandComponent;
class UPWPlayerActionComponent;
class UPWPlayerCaptureComponent;
class UPWPlayerClimbComponent;
class UPWPlayerEquipmentComponent;
class UPWPlayerInteractionComponent;
class UPWPlayerInventoryLinkComponent;
class UPWPlayerMountComponent;
class UPWPlayerPrimaryActionComponent;
class UPWPlayerSkillComponent;
class UPWPlayerStatComponent;

UCLASS()
class PALWORLD_API APWPlayerCharacter : public ACharacter, public IPW_ItemReceiver
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
	void StartPrimaryAction();
	void StopPrimaryAction();
	void Interact();
	bool StartAim();
	void StopAim();
	void SelectNextEquipmentSlot();
	void SelectPreviousEquipmentSlot();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReceiveItem_Implementation(FName ItemId, int32 Count) override;

public:
	UFUNCTION(BlueprintPure, Category = "Player|Movement")
	bool IsSprinting() const;

	bool ShouldDrainSprintStamina() const;

	UFUNCTION(BlueprintPure, Category = "Player|Movement")
	bool IsRolling() const;

	UFUNCTION(BlueprintPure, Category = "Player|Aim")
	bool IsAiming() const { return bIsAiming; }

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
	UPWPlayerPrimaryActionComponent* GetPrimaryActionComponent() const { return PrimaryActionComponent; }
	UPWPlayerEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }
	UPWPlayerInventoryLinkComponent* GetInventoryLinkComponent() const { return InventoryLinkComponent; }
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
	TObjectPtr<UPWPlayerPrimaryActionComponent> PrimaryActionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Player|Components")
	TObjectPtr<UPWPlayerEquipmentComponent> EquipmentComponent;

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

	UPROPERTY(EditDefaultsOnly, Category = "Player|Camera", meta = (ClampMin = "0.0"))
	float DefaultCameraArmLength = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Camera", meta = (ClampMin = "0.0"))
	float AimCameraArmLength = 350.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Camera", meta = (ClampMin = "0.0"))
	float SprintCameraArmLength = 460.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Camera", meta = (ClampMin = "0.0"))
	float ClimbCameraArmLength = 470.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Camera", meta = (ClampMin = "0.0"))
	float CameraZoomInterpSpeed = 12.f;

	UPROPERTY(ReplicatedUsing = OnRep_IsSprinting)
	bool bIsSprinting = false;

	UPROPERTY(ReplicatedUsing = OnRep_IsAiming)
	bool bIsAiming = false;

	// 달리기는 커스텀 속도 상태라 CharacterMovement 기본 이동과 별도로 복제한다.
	UFUNCTION()
	void OnRep_IsSprinting();

	UFUNCTION()
	void OnRep_IsAiming();

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewIsSprinting);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bNewIsAiming);

	bool CanStartSprint() const;
	void SetSprinting(bool bNewIsSprinting);
	bool CanStartAim() const;
	void SetAiming(bool bNewIsAiming);
	bool IsSprintMovementActive() const;
	void ApplyMovementSpeed();
	void ApplyRotationMode();
	void FacePrimaryActionDirection();
	void ApplyPrimaryActionFacing(const FVector& RequestedDirection);
	void UpdateAimRotation();
	float GetTargetCameraArmLength() const;
	void UpdateCameraArmLength(float DeltaSeconds);

	UFUNCTION(Server, Reliable)
	void ServerFacePrimaryActionDirection(FVector_NetQuantizeNormal RequestedDirection);
};
