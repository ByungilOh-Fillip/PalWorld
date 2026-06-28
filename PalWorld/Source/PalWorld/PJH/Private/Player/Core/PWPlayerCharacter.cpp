// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Core/PWPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPalCommandComponent.h"
#include "Player/Components/PWPlayerActionComponent.h"
#include "Player/Components/PWPlayerCaptureComponent.h"
#include "Player/Components/PWPlayerCombatComponent.h"
#include "Player/Components/PWPlayerGatherComponent.h"
#include "Player/Components/PWPlayerInteractionComponent.h"
#include "Player/Components/PWPlayerInventoryLinkComponent.h"
#include "Player/Components/PWPlayerMountComponent.h"
#include "Player/Components/PWPlayerSkillComponent.h"
#include "Player/Components/PWPlayerStatComponent.h"

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
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchedWalkSpeed;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	ActionComponent = CreateDefaultSubobject<UPWPlayerActionComponent>(TEXT("ActionComponent"));
	StatComponent = CreateDefaultSubobject<UPWPlayerStatComponent>(TEXT("StatComponent"));
	CombatComponent = CreateDefaultSubobject<UPWPlayerCombatComponent>(TEXT("CombatComponent"));
	GatherComponent = CreateDefaultSubobject<UPWPlayerGatherComponent>(TEXT("GatherComponent"));
	SkillComponent = CreateDefaultSubobject<UPWPlayerSkillComponent>(TEXT("SkillComponent"));
	PalCommandComponent = CreateDefaultSubobject<UPWPalCommandComponent>(TEXT("PalCommandComponent"));
	InteractionComponent = CreateDefaultSubobject<UPWPlayerInteractionComponent>(TEXT("InteractionComponent"));
	InventoryLinkComponent = CreateDefaultSubobject<UPWPlayerInventoryLinkComponent>(TEXT("InventoryLinkComponent"));
	CaptureComponent = CreateDefaultSubobject<UPWPlayerCaptureComponent>(TEXT("CaptureComponent"));
	MountComponent = CreateDefaultSubobject<UPWPlayerMountComponent>(TEXT("MountComponent"));
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

// 이 입력들은 CharacterMovement가 클라 예측/서버 보정을 기본으로 처리한다.
void APWPlayerCharacter::Move(const FVector2D& MovementVector)
{
	if (!Controller || IsRolling() || MovementVector.IsNearlyZero())
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
	if (IsRolling())
	{
		return;
	}

	Jump();
}

void APWPlayerCharacter::StopJump()
{
	StopJumping();
}

void APWPlayerCharacter::StartSprint()
{
	if (!CanStartSprint())
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

// Crouch/UnCrouch는 ACharacter의 기본 복제 crouch 상태를 사용한다.
void APWPlayerCharacter::StartCrouch()
{
	if (IsRolling())
	{
		return;
	}

	StopSprint();
	Crouch();
}

void APWPlayerCharacter::StopCrouch()
{
	UnCrouch();
}

void APWPlayerCharacter::StartRoll()
{
	if (ActionComponent)
	{
		ActionComponent->TryStartRoll();
	}
}

void APWPlayerCharacter::StartGather()
{
	if (GatherComponent)
	{
		GatherComponent->TryGatherFromView();
	}
}

bool APWPlayerCharacter::IsRolling() const
{
	return ActionComponent && ActionComponent->IsRolling();
}

// --------------------
// 커스텀 복제 이동 상태
// --------------------

void APWPlayerCharacter::OnRep_IsSprinting()
{
	ApplyMovementSpeed();
}

void APWPlayerCharacter::ServerSetSprinting_Implementation(bool bNewIsSprinting)
{
	if (bNewIsSprinting && !CanStartSprint())
	{
		SetSprinting(false);
		return;
	}

	SetSprinting(bNewIsSprinting);
}

bool APWPlayerCharacter::CanStartSprint() const
{
	return !bIsCrouched
		&& !IsRolling()
		&& (!StatComponent || StatComponent->CanStartSprint());
}

void APWPlayerCharacter::SetSprinting(bool bNewIsSprinting)
{
	if (bNewIsSprinting && !CanStartSprint())
	{
		bNewIsSprinting = false;
	}

	if (bIsSprinting == bNewIsSprinting)
	{
		return;
	}

	bIsSprinting = bNewIsSprinting;
	ApplyMovementSpeed();

	if (HasAuthority() && StatComponent)
	{
		StatComponent->SetSprintDrainActive(bIsSprinting);
	}
}

void APWPlayerCharacter::ApplyMovementSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchedWalkSpeed;
}
