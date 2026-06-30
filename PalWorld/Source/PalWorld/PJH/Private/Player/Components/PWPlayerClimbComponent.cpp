// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerClimbComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPlayerStatComponent.h"
#include "Player/Core/PWPlayerCharacter.h"

UPWPlayerClimbComponent::UPWPlayerClimbComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UPWPlayerClimbComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedPlayerCharacter = Cast<APWPlayerCharacter>(GetOwner());
}

void UPWPlayerClimbComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateClimb(DeltaTime);
}

void UPWPlayerClimbComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPWPlayerClimbComponent, bIsClimbing);
	DOREPLIFETIME(UPWPlayerClimbComponent, bIsClimbTopOut);
	DOREPLIFETIME(UPWPlayerClimbComponent, WallNormal);
	DOREPLIFETIME(UPWPlayerClimbComponent, ClimbInputX);
	DOREPLIFETIME(UPWPlayerClimbComponent, ClimbInputY);
}

bool UPWPlayerClimbComponent::TryStartClimb()
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !CanStartClimb())
	{
		return false;
	}

	FHitResult WallHit;
	if (!FindClimbableWall(WallHit))
	{
		return false;
	}

	if (!PlayerCharacter->HasAuthority())
	{
		const UWorld* World = GetWorld();
		const float CurrentTime = World ? World->GetTimeSeconds() : 0.f;
		if (CurrentTime - LastClimbRequestTime < ClimbRequestCooldown)
		{
			return false;
		}

		LastClimbRequestTime = CurrentTime;
		ServerTryStartClimb();
		return true;
	}

	StartClimbServer(WallHit.ImpactNormal);
	return true;
}

void UPWPlayerClimbComponent::StopClimb(bool bLaunchOff)
{
	if (!bIsClimbing && !bIsClimbTopOut)
	{
		return;
	}

	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (PlayerCharacter && !PlayerCharacter->HasAuthority())
	{
		ServerStopClimb(bLaunchOff);
	}

	StopClimbInternal(bLaunchOff);
}

void UPWPlayerClimbComponent::SetClimbInput(const FVector2D& MovementVector)
{
	if (!bIsClimbing)
	{
		return;
	}

	const float NewClimbInputX = FMath::Clamp(MovementVector.X, -1.f, 1.f);
	const float NewClimbInputY = FMath::Clamp(MovementVector.Y, -1.f, 1.f);
	const bool bInputChanged = !FMath::IsNearlyEqual(ClimbInputX, NewClimbInputX)
		|| !FMath::IsNearlyEqual(ClimbInputY, NewClimbInputY);

	SetReplicatedClimbInput(NewClimbInputX, NewClimbInputY);

	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (bInputChanged && PlayerCharacter && !PlayerCharacter->HasAuthority())
	{
		ServerSetClimbInput(ClimbInputX, ClimbInputY);
	}

	MoveAlongWall();
}

float UPWPlayerClimbComponent::GetWallClimbVerticalSpeed() const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	return bIsClimbing && PlayerCharacter ? PlayerCharacter->GetVelocity().Z : 0.f;
}

float UPWPlayerClimbComponent::GetWallClimbHorizontalSpeed() const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	return bIsClimbing && PlayerCharacter
		? FVector::DotProduct(PlayerCharacter->GetVelocity(), GetClimbRightVector())
		: 0.f;
}

float UPWPlayerClimbComponent::GetWallClimbHorizontalBlendValue() const
{
	return bIsClimbing ? ClimbInputX : 0.f;
}

float UPWPlayerClimbComponent::GetWallClimbVerticalBlendValue() const
{
	return bIsClimbing ? ClimbInputY : 0.f;
}

// --------------------
// 네트워크 진입점
// --------------------

void UPWPlayerClimbComponent::OnRep_IsClimbing()
{
	ApplyClimbMovementMode();

	if (bIsClimbing)
	{
		ApplyClimbFacing();
		return;
	}

	SetReplicatedClimbInput(0.f, 0.f);
}

void UPWPlayerClimbComponent::OnRep_WallNormal()
{
	ApplyClimbFacing();
}

void UPWPlayerClimbComponent::ServerTryStartClimb_Implementation()
{
	if (!CanStartClimb())
	{
		return;
	}

	FHitResult WallHit;
	if (!FindClimbableWall(WallHit))
	{
		return;
	}

	StartClimbServer(WallHit.ImpactNormal);
}

void UPWPlayerClimbComponent::ServerStopClimb_Implementation(bool bLaunchOff)
{
	StopClimbInternal(bLaunchOff);
}

void UPWPlayerClimbComponent::ServerSetClimbInput_Implementation(float NewClimbInputX, float NewClimbInputY)
{
	if (!bIsClimbing)
	{
		return;
	}

	SetReplicatedClimbInput(
		FMath::Clamp(NewClimbInputX, -1.f, 1.f),
		FMath::Clamp(NewClimbInputY, -1.f, 1.f)
	);
}

// --------------------
// 공통 검증/상태
// --------------------

APWPlayerCharacter* UPWPlayerClimbComponent::GetPlayerCharacter() const
{
	return CachedPlayerCharacter ? CachedPlayerCharacter.Get() : Cast<APWPlayerCharacter>(GetOwner());
}

UCharacterMovementComponent* UPWPlayerClimbComponent::GetMovementComponent() const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	return PlayerCharacter ? PlayerCharacter->GetCharacterMovement() : nullptr;
}

bool UPWPlayerClimbComponent::CanStartClimb() const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	const UCharacterMovementComponent* MovementComponent = GetMovementComponent();
	const UPWPlayerStatComponent* StatComponent = PlayerCharacter ? PlayerCharacter->GetStatComponent() : nullptr;

	return PlayerCharacter
		&& !bIsClimbing
		&& !bIsClimbTopOut
		&& !PlayerCharacter->IsRolling()
		&& MovementComponent
		&& !MovementComponent->IsCrouching()
		&& !MovementComponent->IsMovingOnGround()
		&& (!StatComponent || StatComponent->HasEnoughStamina(MinClimbStartStamina));
}

bool UPWPlayerClimbComponent::HasClimbMoveInput() const
{
	return !FMath::IsNearlyZero(ClimbInputX) || !FMath::IsNearlyZero(ClimbInputY);
}

bool UPWPlayerClimbComponent::ShouldDrainClimbStamina() const
{
	const UCharacterMovementComponent* MovementComponent = GetMovementComponent();
	if (!bIsClimbing || !HasClimbMoveInput() || !MovementComponent)
	{
		return false;
	}

	// 입력만 누른 상태가 아니라 실제로 이동 결과가 있을 때만 스태미나를 소모한다.
	return MovementComponent->Velocity.SizeSquared() > FMath::Square(MinClimbStaminaDrainSpeed);
}

bool UPWPlayerClimbComponent::ShouldTryMantleFromClimb() const
{
	return bIsClimbing && ClimbInputY > MantleInputThreshold;
}

bool UPWPlayerClimbComponent::FindClimbableWall(FHitResult& OutHit) const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	const UWorld* World = GetWorld();
	if (!PlayerCharacter || !World)
	{
		return false;
	}

	const FVector TraceStart = PlayerCharacter->GetActorLocation() + FVector(0.f, 0.f, 40.f);
	const FVector TraceEnd = TraceStart + PlayerCharacter->GetActorForwardVector() * ClimbTraceDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWClimbTrace), false);
	QueryParams.AddIgnoredActor(PlayerCharacter);

	const FCollisionShape TraceShape = FCollisionShape::MakeSphere(ClimbTraceRadius);
	if (!World->SweepSingleByChannel(OutHit, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, TraceShape, QueryParams))
	{
		return false;
	}

	// 90도는 완전 수직 벽, 70도는 약간 누운 벽까지 허용하는 기준이다.
	const float MaxAllowedNormalZ = FMath::Cos(FMath::DegreesToRadians(MinClimbSurfaceAngleDegrees));
	return OutHit.bBlockingHit
		&& FMath::Abs(OutHit.ImpactNormal.Z) <= MaxAllowedNormalZ
		&& IsClimbableSurface(OutHit);
}

bool UPWPlayerClimbComponent::IsClimbableSurface(const FHitResult& WallHit) const
{
	if (!bRequireClimbableSurfaceTag || ClimbableSurfaceTag.IsNone())
	{
		return true;
	}

	const UPrimitiveComponent* WallComponent = WallHit.GetComponent();
	const AActor* WallActor = WallHit.GetActor();
	return (WallComponent && WallComponent->ComponentHasTag(ClimbableSurfaceTag))
		|| (WallActor && WallActor->ActorHasTag(ClimbableSurfaceTag));
}

bool UPWPlayerClimbComponent::IsClimbTouchingGround() const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	const UWorld* World = GetWorld();
	const UCapsuleComponent* CharacterCapsule = PlayerCharacter ? PlayerCharacter->GetCapsuleComponent() : nullptr;
	if (!PlayerCharacter || !World || !CharacterCapsule)
	{
		return false;
	}

	const FVector TraceStart = PlayerCharacter->GetActorLocation();
	const FVector TraceEnd = TraceStart - FVector::UpVector * (CharacterCapsule->GetScaledCapsuleHalfHeight() + 8.f);
	const FCollisionShape TraceShape = FCollisionShape::MakeSphere(CharacterCapsule->GetScaledCapsuleRadius() * 0.85f);

	FHitResult GroundHit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWClimbGroundTrace), false);
	QueryParams.AddIgnoredActor(PlayerCharacter);

	if (!World->SweepSingleByChannel(GroundHit, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, TraceShape, QueryParams))
	{
		return false;
	}

	return GroundHit.bBlockingHit && GroundHit.ImpactNormal.Z > 0.5f;
}

bool UPWPlayerClimbComponent::TryMantleFromClimb()
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	UCharacterMovementComponent* MovementComponent = GetMovementComponent();
	const UWorld* World = GetWorld();
	const UCapsuleComponent* CharacterCapsule = PlayerCharacter ? PlayerCharacter->GetCapsuleComponent() : nullptr;
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || WallNormal.IsNearlyZero() || !World || !CharacterCapsule || !MovementComponent)
	{
		return false;
	}

	const FVector TowardWall = -WallNormal.GetSafeNormal();
	const float CapsuleRadius = CharacterCapsule->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = CharacterCapsule->GetScaledCapsuleHalfHeight();
	const FVector TraceStart = PlayerCharacter->GetActorLocation()
		+ FVector::UpVector * ClimbLedgeUpProbe
		+ TowardWall * (CapsuleRadius + ClimbLedgeForwardProbe);
	const FVector TraceEnd = TraceStart - FVector::UpVector * ClimbLedgeDownProbe;

	FHitResult LedgeHit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWClimbLedgeTrace), false);
	QueryParams.AddIgnoredActor(PlayerCharacter);

	const FCollisionShape TraceShape = FCollisionShape::MakeSphere(CapsuleRadius * 0.5f);
	if (!World->SweepSingleByChannel(LedgeHit, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, TraceShape, QueryParams))
	{
		return false;
	}

	if (!LedgeHit.bBlockingHit || LedgeHit.ImpactNormal.Z <= 0.6f)
	{
		return false;
	}

	// 지금은 즉시 올려놓고, 추후 이 함수 안에서 Motion Warping/Root Motion TopOut으로 교체한다.
	const FVector TargetLocation = LedgeHit.ImpactPoint + FVector::UpVector * (CapsuleHalfHeight + ClimbLedgeSnapOffset);
	const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	if (World->OverlapBlockingTestByProfile(
		TargetLocation,
		PlayerCharacter->GetActorQuat(),
		CharacterCapsule->GetCollisionProfileName(),
		CapsuleShape,
		QueryParams))
	{
		return false;
	}

	FHitResult MoveHit;
	if (!PlayerCharacter->SetActorLocation(TargetLocation, true, &MoveHit))
	{
		return false;
	}

	bIsClimbing = false;
	bIsClimbTopOut = false;
	WallNormal = FVector::ZeroVector;
	SetReplicatedClimbInput(0.f, 0.f);
	ApplyClimbMovementMode();
	MovementComponent->SetMovementMode(MOVE_Walking);
	MovementComponent->Velocity = FVector::ZeroVector;
	PlayerCharacter->ForceNetUpdate();
	return true;
}

// --------------------
// 서버 권한 상태 변경
// --------------------

void UPWPlayerClimbComponent::StartClimbServer(const FVector& InWallNormal)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || bIsClimbing)
	{
		return;
	}

	PlayerCharacter->StopSprint();
	PlayerCharacter->StopJumping();

	bIsClimbing = true;
	bIsClimbTopOut = false;
	WallNormal = InWallNormal.GetSafeNormal();
	SetReplicatedClimbInput(0.f, 0.f);
	ApplyClimbMovementMode();
	ApplyClimbFacing();
	PlayerCharacter->ForceNetUpdate();
}

void UPWPlayerClimbComponent::StopClimbInternal(bool bLaunchOff)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || (!bIsClimbing && !bIsClimbTopOut))
	{
		return;
	}

	const FVector LaunchNormal = WallNormal.IsNearlyZero()
		? PlayerCharacter->GetActorForwardVector() * -1.f
		: WallNormal.GetSafeNormal();

	bIsClimbing = false;
	bIsClimbTopOut = false;
	WallNormal = FVector::ZeroVector;
	SetReplicatedClimbInput(0.f, 0.f);
	ApplyClimbMovementMode();
	PlayerCharacter->ForceNetUpdate();

	if (bLaunchOff)
	{
		const FVector LaunchVelocity = LaunchNormal * ClimbJumpAwayStrength + FVector::UpVector * ClimbJumpUpStrength;
		PlayerCharacter->LaunchCharacter(LaunchVelocity, true, true);
	}
}

// --------------------
// 이동/감지 처리
// --------------------

void UPWPlayerClimbComponent::CacheMovementValues(UCharacterMovementComponent& MovementComponent)
{
	if (bHasCachedMovementValues)
	{
		return;
	}

	CachedMaxFlySpeed = MovementComponent.MaxFlySpeed;
	CachedBrakingDecelerationFlying = MovementComponent.BrakingDecelerationFlying;
	bCachedOrientRotationToMovement = MovementComponent.bOrientRotationToMovement;
	bHasCachedMovementValues = true;
}

void UPWPlayerClimbComponent::RestoreMovementValues(UCharacterMovementComponent& MovementComponent)
{
	if (!bHasCachedMovementValues)
	{
		return;
	}

	MovementComponent.MaxFlySpeed = CachedMaxFlySpeed;
	MovementComponent.BrakingDecelerationFlying = CachedBrakingDecelerationFlying;
	MovementComponent.bOrientRotationToMovement = bCachedOrientRotationToMovement;
	bHasCachedMovementValues = false;
}

void UPWPlayerClimbComponent::ApplyClimbMovementMode()
{
	UCharacterMovementComponent* MovementComponent = GetMovementComponent();
	if (!MovementComponent)
	{
		return;
	}

	if (bIsClimbing || bIsClimbTopOut)
	{
		CacheMovementValues(*MovementComponent);
		MovementComponent->SetMovementMode(MOVE_Flying);
		MovementComponent->Velocity = FVector::ZeroVector;
		MovementComponent->MaxFlySpeed = ClimbSpeed;
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->BrakingDecelerationFlying = ClimbSpeed * 6.f;
		return;
	}

	RestoreMovementValues(*MovementComponent);
	MovementComponent->SetMovementMode(IsClimbTouchingGround() ? MOVE_Walking : MOVE_Falling);
}

void UPWPlayerClimbComponent::ApplyClimbFacing()
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !bIsClimbing || WallNormal.IsNearlyZero())
	{
		return;
	}

	const FRotator WallFacingRotation(0.f, (-WallNormal.GetSafeNormal()).Rotation().Yaw, 0.f);
	PlayerCharacter->SetActorRotation(WallFacingRotation);
}

void UPWPlayerClimbComponent::MoveAlongWall()
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !bIsClimbing || !HasClimbMoveInput())
	{
		return;
	}

	PlayerCharacter->AddMovementInput(FVector::UpVector, ClimbInputY);
	PlayerCharacter->AddMovementInput(GetClimbRightVector(), ClimbInputX);
}

void UPWPlayerClimbComponent::UpdateClimb(float DeltaSeconds)
{
	if (!bIsClimbing)
	{
		return;
	}

	ApplyClimbFacing();

	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		return;
	}

	if (ShouldTryMantleFromClimb() && TryMantleFromClimb())
	{
		return;
	}

	FHitResult WallHit;
	if (!FindClimbableWall(WallHit))
	{
		if (TryMantleFromClimb())
		{
			return;
		}

		StopClimbInternal(false);
		return;
	}

	WallNormal = WallHit.ImpactNormal.GetSafeNormal();
	MaintainClimbDistance(WallHit, DeltaSeconds);

	const UCharacterMovementComponent* MovementComponent = GetMovementComponent();
	if (MovementComponent && IsClimbTouchingGround() && MovementComponent->Velocity.Z <= 0.f)
	{
		StopClimbInternal(false);
		return;
	}

	UPWPlayerStatComponent* StatComponent = PlayerCharacter->GetStatComponent();
	if (StatComponent
		&& ShouldDrainClimbStamina()
		&& !StatComponent->TryConsumeStamina(ClimbStaminaDrainPerSecond * DeltaSeconds))
	{
		StopClimbInternal(false);
	}
}

void UPWPlayerClimbComponent::MaintainClimbDistance(const FHitResult& WallHit, float DeltaSeconds)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || ClimbDesiredDistance <= 0.f || ClimbDistanceCorrectionSpeed <= 0.f)
	{
		return;
	}

	const FVector SafeWallNormal = WallHit.ImpactNormal.GetSafeNormal();
	const FVector CurrentLocation = PlayerCharacter->GetActorLocation();
	FVector TargetLocation = WallHit.ImpactPoint + SafeWallNormal * ClimbDesiredDistance;
	TargetLocation.Z = CurrentLocation.Z;

	const FVector CorrectionDelta = TargetLocation - CurrentLocation;
	const FVector HorizontalCorrectionDelta(CorrectionDelta.X, CorrectionDelta.Y, 0.f);
	if (HorizontalCorrectionDelta.SizeSquared() <= FMath::Square(1.f))
	{
		return;
	}

	const float MaxCorrection = ClimbDistanceCorrectionSpeed * DeltaSeconds;
	PlayerCharacter->AddActorWorldOffset(HorizontalCorrectionDelta.GetClampedToMaxSize(MaxCorrection), true);
}

void UPWPlayerClimbComponent::SetReplicatedClimbInput(float NewClimbInputX, float NewClimbInputY)
{
	ClimbInputX = FMath::Clamp(NewClimbInputX, -1.f, 1.f);
	ClimbInputY = FMath::Clamp(NewClimbInputY, -1.f, 1.f);

	if (FMath::IsNearlyZero(ClimbInputX))
	{
		ClimbInputX = 0.f;
	}

	if (FMath::IsNearlyZero(ClimbInputY))
	{
		ClimbInputY = 0.f;
	}
}

FVector UPWPlayerClimbComponent::GetClimbRightVector() const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	const FVector FallbackNormal = PlayerCharacter ? -PlayerCharacter->GetActorForwardVector() : FVector::ForwardVector;
	const FVector SafeWallNormal = WallNormal.IsNearlyZero() ? FallbackNormal : WallNormal.GetSafeNormal();
	return FVector::CrossProduct(SafeWallNormal, FVector::UpVector).GetSafeNormal();
}
