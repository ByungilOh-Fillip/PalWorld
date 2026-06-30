// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerActionComponent.h"

#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPlayerStatComponent.h"
#include "Player/Core/PWPlayerCharacter.h"

UPWPlayerActionComponent::UPWPlayerActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPWPlayerActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPWPlayerActionComponent, CurrentActionState);
}

// 외부 입력 진입점. 클라는 액션을 요청만 하고, 실제 검증은 서버가 다시 수행한다.
void UPWPlayerActionComponent::TryStartRoll()
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!CanStartRoll())
	{
		return;
	}

	const FVector RequestedDirection = GetRollDirection();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerRequestStartRoll(FVector_NetQuantizeNormal(RequestedDirection));
		return;
	}

	StartRollAuthority(RequestedDirection);
}

void UPWPlayerActionComponent::SetRollMontage(UAnimMontage* InRollMontage)
{
	RollMontage = InRollMontage;
}

void UPWPlayerActionComponent::SetRollTuning(float InRollDistance, float InRollMovementDuration, bool bInUseCodeDrivenMovement)
{
	RollDistance = InRollDistance;
	RollMovementDuration = FMath::Max(InRollMovementDuration, 0.05f);
	bUseCodeDrivenRollMovement = bInUseCodeDrivenMovement;
}

// --------------------
// 구르기 네트워크 흐름
// --------------------

void UPWPlayerActionComponent::ServerRequestStartRoll_Implementation(FVector_NetQuantizeNormal RequestedDirection)
{
	StartRollAuthority(RequestedDirection);
}

void UPWPlayerActionComponent::MulticastStartRollVisuals_Implementation(FRotator RollFacingRotation)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		return;
	}

	// 상태는 서버가 소유하고, 멀티캐스트는 모든 클라의 몽타주 시작 프레임을 맞춘다.
	CurrentActionState = EPWPlayerActionState::Rolling;
	PlayerCharacter->SetActorRotation(RollFacingRotation);
	PlayRollMontage();
}

// --------------------
// 공통 검증/헬퍼
// --------------------

APWPlayerCharacter* UPWPlayerActionComponent::GetPlayerCharacter() const
{
	return Cast<APWPlayerCharacter>(GetOwner());
}

void UPWPlayerActionComponent::OnRep_CurrentActionState()
{
	// 현재는 AnimInstance가 매 프레임 상태를 읽는다. 나중에 UI/효과가 필요하면 여기서 분기한다.
}

bool UPWPlayerActionComponent::CanStartAction(EPWPlayerActionState RequestedActionState) const
{
	return RequestedActionState != EPWPlayerActionState::None
		&& CurrentActionState == EPWPlayerActionState::None;
}

bool UPWPlayerActionComponent::TryStartActionAuthority(EPWPlayerActionState RequestedActionState)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || !CanStartAction(RequestedActionState))
	{
		return false;
	}

	SetActionStateAuthority(RequestedActionState);
	return true;
}

void UPWPlayerActionComponent::FinishActionAuthority(EPWPlayerActionState FinishedActionState)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		return;
	}

	if (CurrentActionState != FinishedActionState)
	{
		return;
	}

	SetActionStateAuthority(EPWPlayerActionState::None);
}

bool UPWPlayerActionComponent::CanStartRoll() const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	const UCharacterMovementComponent* MovementComponent = PlayerCharacter ? PlayerCharacter->GetCharacterMovement() : nullptr;
	const UPWPlayerStatComponent* StatComponent = PlayerCharacter ? PlayerCharacter->GetStatComponent() : nullptr;

	return PlayerCharacter
		&& RollMontage
		&& CanStartAction(EPWPlayerActionState::Rolling)
		&& (!StatComponent || StatComponent->HasEnoughStamina(StatComponent->GetRollStaminaCost()))
		&& MovementComponent
		&& !MovementComponent->IsCrouching()
		&& MovementComponent->IsMovingOnGround();
}

float UPWPlayerActionComponent::GetRollDuration() const
{
	return RollMontage ? RollMontage->GetPlayLength() : RollFallbackDuration;
}

FVector UPWPlayerActionComponent::GetRollDirection() const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	const UCharacterMovementComponent* MovementComponent = PlayerCharacter ? PlayerCharacter->GetCharacterMovement() : nullptr;
	FVector Direction = MovementComponent ? MovementComponent->GetCurrentAcceleration() : FVector::ZeroVector;
	Direction.Z = 0.f;

	if (Direction.IsNearlyZero() && PlayerCharacter)
	{
		Direction = PlayerCharacter->GetVelocity();
		Direction.Z = 0.f;
	}

	if (Direction.IsNearlyZero() && PlayerCharacter)
	{
		Direction = PlayerCharacter->GetActorForwardVector();
		Direction.Z = 0.f;
	}

	return Direction.GetSafeNormal2D();
}

FVector UPWPlayerActionComponent::ResolveRollDirection(const FVector& RequestedDirection) const
{
	FVector Direction = RequestedDirection;
	Direction.Z = 0.f;

	if (!Direction.IsNearlyZero())
	{
		return Direction.GetSafeNormal2D();
	}

	return GetRollDirection();
}

// --------------------
// 서버 권한 구르기 흐름
// --------------------

void UPWPlayerActionComponent::SetActionStateAuthority(EPWPlayerActionState NewActionState)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		return;
	}

	if (CurrentActionState == NewActionState)
	{
		return;
	}

	CurrentActionState = NewActionState;
	PlayerCharacter->ForceNetUpdate();
}

void UPWPlayerActionComponent::StartRollAuthority(const FVector& RequestedDirection)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || !CanStartRoll())
	{
		return;
	}

	UPWPlayerStatComponent* StatComponent = PlayerCharacter->GetStatComponent();
	if (StatComponent && !StatComponent->TryConsumeStamina(StatComponent->GetRollStaminaCost()))
	{
		return;
	}

	PlayerCharacter->StopSprint();

	const float MontageDuration = GetRollDuration();
	const FVector RollDirection = ResolveRollDirection(RequestedDirection);
	const FRotator RollFacingRotation(0.f, RollDirection.Rotation().Yaw, 0.f);

	PlayerCharacter->SetActorRotation(RollFacingRotation);
	if (bUseCodeDrivenRollMovement)
	{
		ApplyRollMovement(RollDirection, RollMovementDuration);
	}

	SetActionStateAuthority(EPWPlayerActionState::Rolling);
	MulticastStartRollVisuals(RollFacingRotation);

	GetWorld()->GetTimerManager().ClearTimer(RollTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(RollTimerHandle, this, &UPWPlayerActionComponent::FinishRoll, MontageDuration, false);
}

void UPWPlayerActionComponent::FinishRoll()
{
	ClearRollMovement();
	SetActionStateAuthority(EPWPlayerActionState::None);
}

// --------------------
// 연출/이동 헬퍼
// --------------------

void UPWPlayerActionComponent::PlayRollMontage()
{
	if (APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		if (RollMontage)
		{
			PlayerCharacter->PlayAnimMontage(RollMontage);
		}
	}
}

void UPWPlayerActionComponent::ApplyRollMovement(const FVector& RollDirection, float RollDuration)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || RollDistance <= 0.f)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	ClearRollMovement();

	const float SafeDuration = FMath::Max(RollDuration, 0.1f);
	const FVector StartLocation = PlayerCharacter->GetActorLocation();
	FVector TargetLocation = StartLocation + RollDirection.GetSafeNormal2D() * RollDistance;
	TargetLocation.Z = StartLocation.Z;

	TSharedPtr<FRootMotionSource_MoveToForce> RollMove = MakeShared<FRootMotionSource_MoveToForce>();
	RollMove->InstanceName = TEXT("PW_RollMove");
	RollMove->AccumulateMode = ERootMotionAccumulateMode::Override;
	RollMove->Priority = FMath::Clamp(RollRootMotionPriority, 1, TNumericLimits<uint16>::Max());
	RollMove->Duration = SafeDuration;
	RollMove->StartLocation = StartLocation;
	RollMove->TargetLocation = TargetLocation;
	RollMove->bRestrictSpeedToExpected = true;
	RollMove->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
	RollMove->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	RollMove->FinishVelocityParams.SetVelocity = FVector::ZeroVector;

	RollRootMotionSourceId = MovementComponent->ApplyRootMotionSource(RollMove);
}

void UPWPlayerActionComponent::ClearRollMovement()
{
	if (RollRootMotionSourceId == static_cast<uint16>(ERootMotionSourceID::Invalid))
	{
		return;
	}

	if (APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter())
	{
		if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
		{
			MovementComponent->RemoveRootMotionSourceByID(RollRootMotionSourceId);
		}
	}

	RollRootMotionSourceId = static_cast<uint16>(ERootMotionSourceID::Invalid);
}
