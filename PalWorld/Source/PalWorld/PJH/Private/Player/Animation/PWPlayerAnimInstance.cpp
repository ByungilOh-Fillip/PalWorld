// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Animation/PWPlayerAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Player/Core/PWPlayerCharacter.h"

void UPWPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CacheOwningCharacter();
}

void UPWPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter)
	{
		CacheOwningCharacter();
	}

	if (!OwningCharacter)
	{
		Character = nullptr;
		MovementComponent = nullptr;
		Velocity = FVector::ZeroVector;
		GroundSpeed = 0.f;
		ShouldMove = false;
		IsFalling = false;
		Direction = 0.f;
		bIsAccelerating = false;
		bIsInAir = false;
		bIsFallingDown = false;
		bIsCrouched = false;
		bIsSprinting = false;
		bIsRolling = false;
		return;
	}

	Character = OwningCharacter;
	MovementComponent = OwningCharacter->GetCharacterMovement();
	Velocity = OwningCharacter->GetVelocity();
	GroundSpeed = Velocity.Size2D();

	if (GroundSpeed > KINDA_SMALL_NUMBER)
	{
		const FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
		const float ForwardAmount = FVector::DotProduct(OwningCharacter->GetActorForwardVector(), NormalizedVelocity);
		const float RightAmount = FVector::DotProduct(OwningCharacter->GetActorRightVector(), NormalizedVelocity);
		Direction = FMath::RadiansToDegrees(FMath::Atan2(RightAmount, ForwardAmount));
	}
	else
	{
		Direction = 0.f;
	}

	bIsAccelerating = MovementComponent && MovementComponent->GetCurrentAcceleration().SizeSquared2D() > KINDA_SMALL_NUMBER;
	ShouldMove = GroundSpeed > 3.f;
	IsFalling = MovementComponent && MovementComponent->IsFalling();
	bIsInAir = IsFalling;
	bIsFallingDown = bIsInAir && Velocity.Z < -KINDA_SMALL_NUMBER;
	bIsCrouched = MovementComponent && MovementComponent->IsCrouching();
	bIsSprinting = OwningCharacter->IsSprinting();
	bIsRolling = OwningCharacter->IsRolling();
}

void UPWPlayerAnimInstance::CacheOwningCharacter()
{
	OwningCharacter = Cast<APWPlayerCharacter>(TryGetPawnOwner());
	Character = OwningCharacter;
	MovementComponent = OwningCharacter ? OwningCharacter->GetCharacterMovement() : nullptr;
}
