// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PWPlayerAnimInstance.generated.h"

class ACharacter;
class APWPlayerCharacter;
class UCharacterMovementComponent;

UCLASS(Blueprintable)
class PALWORLD_API UPWPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation")
	TObjectPtr<APWPlayerCharacter> OwningCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<ACharacter> Character;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Essential Movement Data")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Essential Movement Data")
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Essential Movement Data")
	bool ShouldMove = false;

	UPROPERTY(BlueprintReadOnly, Category = "Essential Movement Data")
	bool IsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	float Z_Offset = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	bool bIsAccelerating = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	bool bIsFallingDown = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	bool bIsCrouched = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	bool bIsSprinting = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	bool bIsRolling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Climb")
	bool bIsClimbing = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Climb")
	float ClimbInputX = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Climb")
	float ClimbInputY = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	bool bIsWallClimbing = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	bool bIsWallClimbTopOut = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	float WallClimbVerticalSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	float WallClimbHorizontalSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	float WallClimbHorizontal = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Animation|Movement")
	float WallClimbVertical = 0.f;

private:
	void CacheOwningCharacter();
};
