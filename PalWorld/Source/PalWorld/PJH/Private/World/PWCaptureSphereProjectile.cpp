// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/PWCaptureSphereProjectile.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "PWPalBase.h"
#include "UObject/ConstructorHelpers.h"

APWCaptureSphereProjectile::APWCaptureSphereProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	SetReplicateMovement(true);

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);
	SphereCollision->InitSphereRadius(22.f);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereCollision->SetCollisionObjectType(ECC_WorldDynamic);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Block);
	SphereCollision->SetNotifyRigidBodyCollision(true);
	SphereCollision->OnComponentHit.AddDynamic(this, &APWCaptureSphereProjectile::HandleProjectileHit);

	VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
	VisualRoot->SetupAttachment(SphereCollision);

	SphereVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereVisual"));
	SphereVisual->SetupAttachment(VisualRoot);
	SphereVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereVisual->SetGenerateOverlapEvents(false);
	SphereVisual->SetSimulatePhysics(false);
	SphereVisual->SetReceivesDecals(false);
	SphereVisual->SetRenderCustomDepth(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultSphereMesh(
		TEXT("/Game/PJH/Data/pal_sphere_palworld/Cleaned/SM_PalSphere_Closed.SM_PalSphere_Closed"));
	if (DefaultSphereMesh.Succeeded())
	{
		ClosedSphereMesh = DefaultSphereMesh.Object;
		SphereVisual->SetStaticMesh(ClosedSphereMesh);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultOpenSphereMesh(
		TEXT("/Game/PJH/Data/pal_sphere_palworld/Cleaned/SM_PalSphere_Open.SM_PalSphere_Open"));
	if (DefaultOpenSphereMesh.Succeeded())
	{
		OpenSphereMesh = DefaultOpenSphereMesh.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultBodyMaterial(
		TEXT("/Game/PJH/Data/pal_sphere_palworld/Cleaned/M_PalSphere_Body_Clean.M_PalSphere_Body_Clean"));
	if (DefaultBodyMaterial.Succeeded())
	{
		BodyMaterial = DefaultBodyMaterial.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultGlowMaterial(
		TEXT("/Game/PJH/Data/pal_sphere_palworld/Cleaned/M_PalSphere_Glow_Clean.M_PalSphere_Glow_Clean"));
	if (DefaultGlowMaterial.Succeeded())
	{
		GlowMaterial = DefaultGlowMaterial.Object;
	}

	ApplyCleanMaterials();

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = SphereCollision;
	ProjectileMovement->InitialSpeed = 1800.f;
	ProjectileMovement->MaxSpeed = 1800.f;
	ProjectileMovement->ProjectileGravityScale = 1.f;
	// 스피어 메쉬는 위/아래 방향이 중요해서 투사체 속도 방향으로 눕히지 않는다.
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.45f;
	ProjectileMovement->Friction = 0.35f;
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = StopBounceSpeed;
	ProjectileMovement->OnProjectileBounce.AddDynamic(this, &APWCaptureSphereProjectile::HandleProjectileBounce);
}

void APWCaptureSphereProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (LifeSeconds > 0.f)
	{
		SetLifeSpan(LifeSeconds);
	}

	if (AActor* OwnerActor = GetOwner())
	{
		SphereCollision->IgnoreActorWhenMoving(OwnerActor, true);
	}
}

void APWCaptureSphereProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ProjectileState == EPWCaptureSphereProjectileState::Flying)
	{
		UpdateFlyingVisual(DeltaSeconds);
	}
	else
	{
		UpdateCaptureSequence(DeltaSeconds);
	}
}

void APWCaptureSphereProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APWCaptureSphereProjectile, ProjectileState);
	DOREPLIFETIME(APWCaptureSphereProjectile, StateElapsedTime);
	DOREPLIFETIME(APWCaptureSphereProjectile, HitReactStartLocation);
	DOREPLIFETIME(APWCaptureSphereProjectile, HitReactTargetLocation);
}

void APWCaptureSphereProjectile::LaunchInDirection(const FVector& Direction)
{
	if (!ProjectileMovement)
	{
		return;
	}

	FVector LaunchDirection = Direction.GetSafeNormal();
	if (LaunchDirection.IsNearlyZero())
	{
		LaunchDirection = GetActorForwardVector();
	}

	ProjectileMovement->Velocity = LaunchDirection * ProjectileMovement->InitialSpeed;
}

void APWCaptureSphereProjectile::HandleProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority() || ProjectileState != EPWCaptureSphereProjectileState::Flying)
	{
		return;
	}

	if (!OtherActor || OtherActor == GetOwner() || OtherActor == this)
	{
		return;
	}

	if (IsCaptureTarget(OtherActor))
	{
		UE_LOG(LogTemp, Display, TEXT("[PWCapture] Sphere hit capture target. Projectile=%s Target=%s"),
			*GetName(),
			*OtherActor->GetName());
		StartHitReact(Hit, OtherActor);
		return;
	}

	// 월드나 일반 오브젝트에 맞았을 때는 ProjectileMovement의 바운스를 그대로 살린다.
}

void APWCaptureSphereProjectile::HandleProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (!HasAuthority() || ProjectileState != EPWCaptureSphereProjectileState::Flying)
	{
		return;
	}

	++WorldBounceCount;

	const float CurrentSpeed = ProjectileMovement ? ProjectileMovement->Velocity.Size() : 0.f;
	UE_LOG(LogTemp, Verbose, TEXT("[PWCapture] Sphere bounced. Projectile=%s Count=%d Speed=%.1f Hit=%s"),
		*GetName(),
		WorldBounceCount,
		CurrentSpeed,
		*GetNameSafe(ImpactResult.GetActor()));

	if (WorldBounceCount >= MaxWorldBounceCount || CurrentSpeed <= StopBounceSpeed)
	{
		if (ProjectileMovement)
		{
			ProjectileMovement->StopMovementImmediately();
			ProjectileMovement->Deactivate();
		}
	}
}

void APWCaptureSphereProjectile::OnRep_ProjectileState()
{
	if (!HasAuthority() && ProjectileState != EPWCaptureSphereProjectileState::Flying)
	{
		StopProjectileMovement();
		if (ProjectileState == EPWCaptureSphereProjectileState::HitReact && HasValidHitReactLocations())
		{
			SetActorLocation(HitReactStartLocation);
		}
	}

	ApplyStateVisual();
}

void APWCaptureSphereProjectile::OnRep_HitReactLocations()
{
	if (HasAuthority() || ProjectileState != EPWCaptureSphereProjectileState::HitReact || !HasValidHitReactLocations())
	{
		return;
	}

	StopProjectileMovement();
	StateElapsedTime = 0.f;
	SetActorLocation(HitReactStartLocation);
}

void APWCaptureSphereProjectile::MulticastStartHitReact_Implementation(
	FVector_NetQuantize StartLocation,
	FVector_NetQuantize TargetLocation)
{
	HitReactStartLocation = StartLocation;
	HitReactTargetLocation = TargetLocation;
	StateElapsedTime = 0.f;

	if (!HasAuthority())
	{
		ProjectileState = EPWCaptureSphereProjectileState::HitReact;
	}

	StopProjectileMovement();
	SetActorLocation(HitReactStartLocation);
	ApplyStateVisual();
}

void APWCaptureSphereProjectile::EnterState(EPWCaptureSphereProjectileState NewState)
{
	if (ProjectileState == NewState)
	{
		return;
	}

	ProjectileState = NewState;
	StateElapsedTime = 0.f;
	ApplyStateVisual();

	if (ProjectileState == EPWCaptureSphereProjectileState::Finished)
	{
		SetLifeSpan(1.f);
	}

	ForceNetUpdate();
}

void APWCaptureSphereProjectile::UpdateCaptureSequence(float DeltaSeconds)
{
	if (ProjectileState == EPWCaptureSphereProjectileState::HitReact
		&& !HasAuthority()
		&& HitReactStartLocation.IsNearlyZero()
		&& HitReactTargetLocation.IsNearlyZero())
	{
		// 서버가 계산한 튕김 시작/목표 위치가 아직 복제되기 전이면 클라에서 원점으로 보간하지 않는다.
		return;
	}

	StateElapsedTime += DeltaSeconds;

	switch (ProjectileState)
	{
	case EPWCaptureSphereProjectileState::HitReact:
	{
		UpdateHitReact(DeltaSeconds);
		break;
	}
	case EPWCaptureSphereProjectileState::Opening:
	{
		const float Alpha = OpenDuration > 0.f ? FMath::Clamp(StateElapsedTime / OpenDuration, 0.f, 1.f) : 1.f;
		VisualRoot->SetRelativeScale3D(FVector(1.f + Alpha * 0.18f));
		if (HasAuthority() && StateElapsedTime >= OpenDuration)
		{
			EnterState(EPWCaptureSphereProjectileState::Closing);
		}
		break;
	}
	case EPWCaptureSphereProjectileState::Closing:
	{
		const float Alpha = CloseDuration > 0.f ? FMath::Clamp(StateElapsedTime / CloseDuration, 0.f, 1.f) : 1.f;
		VisualRoot->SetRelativeScale3D(FVector(1.18f - Alpha * 0.18f));
		if (HasAuthority() && StateElapsedTime >= CloseDuration)
		{
			EnterState(EPWCaptureSphereProjectileState::Capturing);
		}
		break;
	}
	case EPWCaptureSphereProjectileState::Capturing:
	{
		const float Shake = FMath::Sin(StateElapsedTime * CaptureShakeFrequency * TWO_PI) * CaptureShakeYawAmplitude;
		VisualRoot->SetRelativeRotation(FRotator(0.f, Shake, 0.f));
		if (HasAuthority() && StateElapsedTime >= CaptureShakeDuration)
		{
			UE_LOG(LogTemp, Display, TEXT("[PWCapture] Capture sequence finished. Projectile=%s Target=%s"),
				*GetName(),
				*GetNameSafe(HitTarget.Get()));
			EnterState(EPWCaptureSphereProjectileState::Finished);
		}
		break;
	}
	default:
		break;
	}
}

void APWCaptureSphereProjectile::StartHitReact(const FHitResult& Hit, AActor* CaptureTarget)
{
	const FVector ImpactVelocity = ProjectileMovement ? ProjectileMovement->Velocity : FVector::ZeroVector;
	StopProjectileMovement();

	HitTarget = CaptureTarget;
	HitReactStartLocation = Hit.ImpactPoint + Hit.ImpactNormal * 12.f;

	const FVector IncomingDirection = ImpactVelocity.GetSafeNormal();
	const FVector BackDirection = (-IncomingDirection).GetSafeNormal();
	const FVector SafeBackDirection = BackDirection.IsNearlyZero() ? Hit.ImpactNormal.GetSafeNormal() : BackDirection;

	HitReactTargetLocation = HitReactStartLocation
		+ SafeBackDirection * HitReactBackDistance
		+ FVector::UpVector * HitReactUpHeight;

	UE_LOG(LogTemp, Display, TEXT("[PWCapture] Sphere hit react. Projectile=%s Target=%s Start=%s TargetLocation=%s UpHeight=%.1f BackDistance=%.1f"),
		*GetName(),
		*GetNameSafe(CaptureTarget),
		*HitReactStartLocation.ToString(),
		*HitReactTargetLocation.ToString(),
		HitReactUpHeight,
		HitReactBackDistance);

	SetActorLocation(HitReactStartLocation);
	EnterState(EPWCaptureSphereProjectileState::HitReact);
	MulticastStartHitReact(HitReactStartLocation, HitReactTargetLocation);
}

void APWCaptureSphereProjectile::UpdateHitReact(float DeltaSeconds)
{
	const float Alpha = HitReactDuration > 0.f ? FMath::Clamp(StateElapsedTime / HitReactDuration, 0.f, 1.f) : 1.f;
	const float EaseAlpha = FMath::Sin(Alpha * HALF_PI);
	const FVector NewLocation = FMath::Lerp(HitReactStartLocation, HitReactTargetLocation, EaseAlpha);
	SetActorLocation(NewLocation);

	if (VisualRoot)
	{
		VisualRoot->AddLocalRotation(FRotator(540.f * DeltaSeconds, 0.f, 180.f * DeltaSeconds));
	}

	if (HasAuthority() && Alpha >= 1.f)
	{
		EnterState(EPWCaptureSphereProjectileState::Opening);
	}
}

void APWCaptureSphereProjectile::UpdateFlyingVisual(float DeltaSeconds)
{
	if (!VisualRoot || !ProjectileMovement)
	{
		return;
	}

	const float SpeedAlpha = ProjectileMovement->Velocity.Size() / FMath::Max(ProjectileMovement->InitialSpeed, 1.f);
	const float SpinThisFrame = FlyingSpinSpeed * SpeedAlpha * DeltaSeconds;
	VisualRoot->AddLocalRotation(FRotator(SpinThisFrame, 0.f, SpinThisFrame * 0.35f));
}

void APWCaptureSphereProjectile::ApplyStateVisual()
{
	if (!SphereVisual)
	{
		return;
	}

	if (ProjectileState == EPWCaptureSphereProjectileState::Opening && OpenSphereMesh)
	{
		SphereVisual->SetStaticMesh(OpenSphereMesh);
	}
	else if (ClosedSphereMesh)
	{
		SphereVisual->SetStaticMesh(ClosedSphereMesh);
	}

	if (ProjectileState != EPWCaptureSphereProjectileState::Capturing && ProjectileState != EPWCaptureSphereProjectileState::HitReact)
	{
		VisualRoot->SetRelativeRotation(FRotator::ZeroRotator);
	}

	ApplyCleanMaterials();
}

void APWCaptureSphereProjectile::ApplyCleanMaterials()
{
	if (!SphereVisual)
	{
		return;
	}

	if (BodyMaterial)
	{
		SphereVisual->SetMaterial(0, BodyMaterial);
	}

	if (GlowMaterial)
	{
		SphereVisual->SetMaterial(1, GlowMaterial);
	}
}

bool APWCaptureSphereProjectile::IsCaptureTarget(AActor* OtherActor) const
{
	return OtherActor && OtherActor->IsA<APWPalBase>();
}

bool APWCaptureSphereProjectile::HasValidHitReactLocations() const
{
	return !HitReactStartLocation.IsNearlyZero() || !HitReactTargetLocation.IsNearlyZero();
}

void APWCaptureSphereProjectile::StopProjectileMovement()
{
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	if (SphereCollision)
	{
		SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
