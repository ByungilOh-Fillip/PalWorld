// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Core/PWPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

APWPlayerCharacter::APWPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void APWPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyMovementSpeed();
}

void APWPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APWPlayerCharacter, bIsSprinting);
}

void APWPlayerCharacter::Move(const FVector2D& MovementVector)
{
	if (!Controller || MovementVector.IsNearlyZero())
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void APWPlayerCharacter::Look(const FVector2D& LookVector)
{
	if (LookVector.IsNearlyZero())
	{
		return;
	}

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void APWPlayerCharacter::StartJump()
{
	Jump();
}

void APWPlayerCharacter::StopJump()
{
	StopJumping();
}

void APWPlayerCharacter::StartSprint()
{
	if (bIsCrouched)
	{
		return;
	}

	SetSprinting(true);

	if (!HasAuthority())
	{
		ServerSetSprinting(true);
	}
}

void APWPlayerCharacter::StopSprint()
{
	SetSprinting(false);

	if (!HasAuthority())
	{
		ServerSetSprinting(false);
	}
}

void APWPlayerCharacter::StartCrouch()
{
	StopSprint();
	Crouch();
}

void APWPlayerCharacter::StopCrouch()
{
	UnCrouch();
}

void APWPlayerCharacter::OnRep_IsSprinting()
{
	ApplyMovementSpeed();
}

void APWPlayerCharacter::ServerSetSprinting_Implementation(bool bNewIsSprinting)
{
	SetSprinting(bNewIsSprinting);
}

void APWPlayerCharacter::SetSprinting(bool bNewIsSprinting)
{
	if (bIsSprinting == bNewIsSprinting)
	{
		return;
	}

	bIsSprinting = bNewIsSprinting;
	ApplyMovementSpeed();
}

void APWPlayerCharacter::ApplyMovementSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
}
