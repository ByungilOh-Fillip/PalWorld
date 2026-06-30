// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Core/PWPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPalCommandComponent.h"
#include "Player/Components/PWPlayerActionComponent.h"
#include "Player/Components/PWPlayerCaptureComponent.h"
#include "Player/Components/PWPlayerClimbComponent.h"
#include "Player/Components/PWPlayerEquipmentComponent.h"
#include "Player/Components/PWPlayerInteractionComponent.h"
#include "Player/Components/PWPlayerInventoryLinkComponent.h"
#include "Player/Components/PWPlayerMountComponent.h"
#include "Player/Components/PWPlayerPrimaryActionComponent.h"
#include "Player/Components/PWPlayerSkillComponent.h"
#include "Player/Components/PWPlayerStatComponent.h"

APWPlayerCharacter::APWPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

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
	CameraBoom->TargetArmLength = DefaultCameraArmLength;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	ActionComponent = CreateDefaultSubobject<UPWPlayerActionComponent>(TEXT("ActionComponent"));
	StatComponent = CreateDefaultSubobject<UPWPlayerStatComponent>(TEXT("StatComponent"));
	PrimaryActionComponent = CreateDefaultSubobject<UPWPlayerPrimaryActionComponent>(TEXT("PrimaryActionComponent"));
	EquipmentComponent = CreateDefaultSubobject<UPWPlayerEquipmentComponent>(TEXT("EquipmentComponent"));
	SkillComponent = CreateDefaultSubobject<UPWPlayerSkillComponent>(TEXT("SkillComponent"));
	PalCommandComponent = CreateDefaultSubobject<UPWPalCommandComponent>(TEXT("PalCommandComponent"));
	InteractionComponent = CreateDefaultSubobject<UPWPlayerInteractionComponent>(TEXT("InteractionComponent"));
	InventoryLinkComponent = CreateDefaultSubobject<UPWPlayerInventoryLinkComponent>(TEXT("InventoryLinkComponent"));
	CaptureComponent = CreateDefaultSubobject<UPWPlayerCaptureComponent>(TEXT("CaptureComponent"));
	MountComponent = CreateDefaultSubobject<UPWPlayerMountComponent>(TEXT("MountComponent"));
	ClimbComponent = CreateDefaultSubobject<UPWPlayerClimbComponent>(TEXT("ClimbComponent"));
}

void APWPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyMovementSpeed();
	ApplyRotationMode();
	UpdateCameraArmLength(0.f);
}

void APWPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateAimRotation();
	UpdateCameraArmLength(DeltaSeconds);
}

void APWPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APWPlayerCharacter, bIsSprinting);
	DOREPLIFETIME(APWPlayerCharacter, bIsAiming);
}

bool APWPlayerCharacter::ReceiveItem_Implementation(FName ItemId, int32 Count)
{
	return InventoryLinkComponent && InventoryLinkComponent->AddItem(ItemId, Count);
}

// 이 입력들은 CharacterMovement가 클라 예측/서버 보정을 기본으로 처리한다.
void APWPlayerCharacter::Move(const FVector2D& MovementVector)
{
	if (!Controller || IsRolling() || IsWallClimbTopOut())
	{
		return;
	}

	if (ClimbComponent && ClimbComponent->IsClimbing())
	{
		ClimbComponent->SetClimbInput(MovementVector);
		return;
	}

	if (MovementVector.IsNearlyZero())
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
	if (IsRolling() || IsWallClimbTopOut())
	{
		return;
	}

	if (ClimbComponent && ClimbComponent->IsClimbing())
	{
		ClimbComponent->StopClimb(true);
		return;
	}

	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent && !MovementComponent->IsMovingOnGround() && ClimbComponent && ClimbComponent->TryStartClimb())
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
	if (IsRolling() || IsWallClimbing() || IsWallClimbTopOut())
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
	if (IsWallClimbing() || IsWallClimbTopOut())
	{
		return;
	}

	if (ActionComponent)
	{
		ActionComponent->TryStartRoll();
	}
}

void APWPlayerCharacter::StartPrimaryAction()
{
	if (IsWallClimbing() || IsWallClimbTopOut())
	{
		return;
	}

	FacePrimaryActionDirection();

	if (PrimaryActionComponent)
	{
		PrimaryActionComponent->TryStartPrimaryAction();
	}
}

bool APWPlayerCharacter::StartAim()
{
	if (!CanStartAim())
	{
		return false;
	}

	StopSprint();
	SetAiming(true);

	if (!HasAuthority())
	{
		ServerSetAiming(true);
	}

	return true;
}

void APWPlayerCharacter::StopAim()
{
	SetAiming(false);

	if (!HasAuthority())
	{
		ServerSetAiming(false);
	}
}

void APWPlayerCharacter::SelectNextEquipmentSlot()
{
	if (EquipmentComponent)
	{
		EquipmentComponent->SelectNextEquipmentSlot();
	}
}

void APWPlayerCharacter::SelectPreviousEquipmentSlot()
{
	if (EquipmentComponent)
	{
		EquipmentComponent->SelectPreviousEquipmentSlot();
	}
}

bool APWPlayerCharacter::IsRolling() const
{
	return ActionComponent && ActionComponent->IsRolling();
}

bool APWPlayerCharacter::IsWallClimbing() const
{
	return ClimbComponent && ClimbComponent->IsClimbing();
}

bool APWPlayerCharacter::IsWallClimbTopOut() const
{
	return ClimbComponent && ClimbComponent->IsClimbTopOut();
}

float APWPlayerCharacter::GetClimbInputX() const
{
	return ClimbComponent ? ClimbComponent->GetClimbInputX() : 0.f;
}

float APWPlayerCharacter::GetClimbInputY() const
{
	return ClimbComponent ? ClimbComponent->GetClimbInputY() : 0.f;
}

float APWPlayerCharacter::GetWallClimbVerticalSpeed() const
{
	return ClimbComponent ? ClimbComponent->GetWallClimbVerticalSpeed() : 0.f;
}

float APWPlayerCharacter::GetWallClimbHorizontalSpeed() const
{
	return ClimbComponent ? ClimbComponent->GetWallClimbHorizontalSpeed() : 0.f;
}

float APWPlayerCharacter::GetWallClimbHorizontalBlendValue() const
{
	return ClimbComponent ? ClimbComponent->GetWallClimbHorizontalBlendValue() : 0.f;
}

float APWPlayerCharacter::GetWallClimbVerticalBlendValue() const
{
	return ClimbComponent ? ClimbComponent->GetWallClimbVerticalBlendValue() : 0.f;
}

bool APWPlayerCharacter::IsSprinting() const
{
	return IsSprintMovementActive();
}

bool APWPlayerCharacter::ShouldDrainSprintStamina() const
{
	return IsSprintMovementActive();
}

// --------------------
// 커스텀 복제 이동 상태
// --------------------

void APWPlayerCharacter::OnRep_IsSprinting()
{
	ApplyMovementSpeed();
}

void APWPlayerCharacter::OnRep_IsAiming()
{
	ApplyRotationMode();
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

void APWPlayerCharacter::ServerSetAiming_Implementation(bool bNewIsAiming)
{
	if (bNewIsAiming && !CanStartAim())
	{
		SetAiming(false);
		return;
	}

	SetAiming(bNewIsAiming);
}

bool APWPlayerCharacter::CanStartSprint() const
{
	return !bIsCrouched
		&& !IsRolling()
		&& !bIsAiming
		&& !IsWallClimbing()
		&& !IsWallClimbTopOut()
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

bool APWPlayerCharacter::CanStartAim() const
{
	return !IsRolling()
		&& !IsWallClimbing()
		&& !IsWallClimbTopOut();
}

void APWPlayerCharacter::SetAiming(bool bNewIsAiming)
{
	if (bNewIsAiming && !CanStartAim())
	{
		bNewIsAiming = false;
	}

	if (bIsAiming == bNewIsAiming)
	{
		return;
	}

	bIsAiming = bNewIsAiming;
	ApplyRotationMode();
}

bool APWPlayerCharacter::IsSprintMovementActive() const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!bIsSprinting || !MovementComponent)
	{
		return false;
	}

	// Shift 입력 자체가 아니라 서버가 계산한 실제 이동 결과를 기준으로 달리기 판정한다.
	return !bIsCrouched
		&& !IsRolling()
		&& !IsWallClimbing()
		&& !IsWallClimbTopOut()
		&& MovementComponent->IsMovingOnGround()
		&& MovementComponent->Velocity.SizeSquared2D() > FMath::Square(MinSprintActiveSpeed);
}

void APWPlayerCharacter::ApplyMovementSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchedWalkSpeed;
}

void APWPlayerCharacter::ApplyRotationMode()
{
	const bool bUseAimRotation = bIsAiming && !IsWallClimbing() && !IsWallClimbTopOut();
	bUseControllerRotationYaw = bUseAimRotation;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = !bUseAimRotation;
	}
}

void APWPlayerCharacter::FacePrimaryActionDirection()
{
	if (!Controller)
	{
		return;
	}

	const FVector ViewDirection = Controller->GetControlRotation().Vector();
	ApplyPrimaryActionFacing(ViewDirection);

	if (!HasAuthority())
	{
		ServerFacePrimaryActionDirection(FVector_NetQuantizeNormal(ViewDirection));
	}
}

void APWPlayerCharacter::ApplyPrimaryActionFacing(const FVector& RequestedDirection)
{
	FVector FlatDirection = RequestedDirection;
	FlatDirection.Z = 0.f;

	if (FlatDirection.IsNearlyZero())
	{
		return;
	}

	// 좌클릭 액션은 조준 상태로 들어가지 않고, 시작 순간만 카메라 Yaw를 바라본다.
	SetActorRotation(FRotator(0.f, FlatDirection.Rotation().Yaw, 0.f));
}

void APWPlayerCharacter::ServerFacePrimaryActionDirection_Implementation(FVector_NetQuantizeNormal RequestedDirection)
{
	if (IsRolling() || IsWallClimbing() || IsWallClimbTopOut())
	{
		return;
	}

	ApplyPrimaryActionFacing(RequestedDirection);
}

void APWPlayerCharacter::UpdateAimRotation()
{
	if (!bIsAiming || !Controller || IsWallClimbing() || IsWallClimbTopOut())
	{
		return;
	}

	// 조준 중에는 이동 방향이 아니라 카메라가 바라보는 Yaw를 캐릭터 정면으로 사용한다.
	const FRotator ControlRotation = Controller->GetControlRotation();
	SetActorRotation(FRotator(0.f, ControlRotation.Yaw, 0.f));
}

float APWPlayerCharacter::GetTargetCameraArmLength() const
{
	if (bIsAiming)
	{
		return AimCameraArmLength;
	}

	if (IsWallClimbing() || IsWallClimbTopOut())
	{
		return ClimbCameraArmLength;
	}

	if (IsSprintMovementActive())
	{
		return SprintCameraArmLength;
	}

	return DefaultCameraArmLength;
}

void APWPlayerCharacter::UpdateCameraArmLength(float DeltaSeconds)
{
	if (!CameraBoom)
	{
		return;
	}

	const float TargetArmLength = GetTargetCameraArmLength();
	if (DeltaSeconds <= 0.f || CameraZoomInterpSpeed <= 0.f)
	{
		CameraBoom->TargetArmLength = TargetArmLength;
		return;
	}

	CameraBoom->TargetArmLength = FMath::FInterpTo(
		CameraBoom->TargetArmLength,
		TargetArmLength,
		DeltaSeconds,
		CameraZoomInterpSpeed);
}
