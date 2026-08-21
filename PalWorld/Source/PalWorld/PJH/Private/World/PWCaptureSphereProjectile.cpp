// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/PWCaptureSphereProjectile.h"

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPlayerCaptureComponent.h"
#include "Player/Components/PWPlayerPalStorageComponent.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "Player/UI/PWCaptureProgressWidget.h"
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

	CaptureProgressWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("CaptureProgressWidget"));
	CaptureProgressWidget->SetupAttachment(SphereCollision);
	CaptureProgressWidget->SetWidgetSpace(EWidgetSpace::Screen);
	CaptureProgressWidget->SetWidgetClass(UPWCaptureProgressWidget::StaticClass());
	CaptureProgressWidget->SetDrawSize(FVector2D(640.f, 640.f));
	CaptureProgressWidget->SetPivot(FVector2D(0.5f, 0.5f));
	CaptureProgressWidget->SetVisibility(false);
	CaptureProgressWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CaptureProgressWidget->SetGenerateOverlapEvents(false);
	CaptureProgressWidget->SetOnlyOwnerSee(false);
	CaptureProgressWidget->SetTickWhenOffscreen(true);

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

void APWCaptureSphereProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideCaptureProgressWidget();
	RestoreHitTargetIfNeeded();

	Super::EndPlay(EndPlayReason);
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
	FVector_NetQuantize TargetLocation,
	APWPalBase* TargetPal,
	float InitialChance,
	float TargetDisplayChance,
	float FailureProgress)
{
	HitReactStartLocation = StartLocation;
	HitReactTargetLocation = TargetLocation;
	CaptureUiTarget = TargetPal;
	CaptureUiInitialChance = InitialChance;
	CaptureUiTargetChance = TargetDisplayChance;
	CaptureFailureProgress = FailureProgress;
	bCaptureUiStarted = false;
	StateElapsedTime = 0.f;

	if (!HasAuthority())
	{
		ProjectileState = EPWCaptureSphereProjectileState::HitReact;
	}

	StopProjectileMovement();
	SetActorLocation(HitReactStartLocation);
	ApplyStateVisual();
	ShowCaptureProgressWidget();
}

void APWCaptureSphereProjectile::MulticastSetHitTargetSuppressed_Implementation(AActor* TargetActor, bool bSuppressed, FTransform RestoreTransform)
{
	if (!IsValid(TargetActor))
	{
		return;
	}

	if (!bSuppressed)
	{
		TargetActor->SetActorTransform(RestoreTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}

	TargetActor->SetActorHiddenInGame(bSuppressed);
	TargetActor->SetActorEnableCollision(!bSuppressed);
	TargetActor->SetActorTickEnabled(!bSuppressed);

	TArray<UActorComponent*> Components;
	TargetActor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component)
		{
			Component->SetComponentTickEnabled(!bSuppressed);
		}
	}
}

void APWCaptureSphereProjectile::EnterState(EPWCaptureSphereProjectileState NewState)
{
	if (ProjectileState == NewState)
	{
		return;
	}

	ProjectileState = NewState;
	StateElapsedTime = 0.f;

	if (HasAuthority() && ProjectileState == EPWCaptureSphereProjectileState::Opening)
	{
		SetHitTargetSuppressed(true);
	}

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
		UpdateCaptureProgressWidget();
		break;
	}
	case EPWCaptureSphereProjectileState::Opening:
	{
		const float Alpha = OpenDuration > 0.f ? FMath::Clamp(StateElapsedTime / OpenDuration, 0.f, 1.f) : 1.f;
		const float Pop = FMath::Sin(Alpha * PI);
		VisualRoot->SetRelativeScale3D(FVector(1.f + Alpha * 0.10f + Pop * 0.28f));
		if (HasAuthority() && StateElapsedTime >= OpenDuration)
		{
			EnterState(EPWCaptureSphereProjectileState::Closing);
		}
		UpdateCaptureProgressWidget();
		break;
	}
	case EPWCaptureSphereProjectileState::Closing:
	{
		const float Alpha = CloseDuration > 0.f ? FMath::Clamp(StateElapsedTime / CloseDuration, 0.f, 1.f) : 1.f;
		const float Impact = FMath::Sin(Alpha * PI);
		VisualRoot->SetRelativeScale3D(FVector(1.10f - Alpha * 0.10f + Impact * 0.10f));
		if (HasAuthority() && StateElapsedTime >= CloseDuration)
		{
			EnterState(EPWCaptureSphereProjectileState::Capturing);
		}
		UpdateCaptureProgressWidget();
		break;
	}
	case EPWCaptureSphereProjectileState::Capturing:
	{
		const float Shake = EvaluateCaptureShakeYaw();
		const float ShakeAlpha = FMath::Clamp(FMath::Abs(Shake) / FMath::Max(CaptureShakeYawAmplitude, 1.f), 0.f, 1.f);
		VisualRoot->SetRelativeRotation(FRotator(0.f, Shake, Shake * 0.25f));
		VisualRoot->SetRelativeScale3D(FVector(1.f + ShakeAlpha * 0.06f));
		const bool bReachedDuration = StateElapsedTime >= CaptureShakeDuration;
		const bool bReachedFailurePoint = !bPendingCaptureSuccess && CalculateCaptureUiProgress() >= CaptureFailureProgress;
		if (HasAuthority() && (bReachedDuration || bReachedFailurePoint))
		{
			UpdateCaptureProgressWidget();
			ResolveCaptureResult();
			HideCaptureProgressWidget();
			EnterState(EPWCaptureSphereProjectileState::Finished);
		}
		else
		{
			UpdateCaptureProgressWidget();
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
	HitTargetOriginalTransform = CaptureTarget ? CaptureTarget->GetActorTransform() : FTransform::Identity;
	PendingCaptureChance = 0.f;
	PendingCaptureRoll = 1.f;
	bPendingCaptureSuccess = false;
	bCaptureResultResolved = false;
	bHitTargetSuppressed = false;

	const APWPlayerCharacter* OwnerCharacter = Cast<APWPlayerCharacter>(GetOwner());
	const UPWPlayerCaptureComponent* CaptureComponent = OwnerCharacter ? OwnerCharacter->GetCaptureComponent() : nullptr;
	if (APWPalBase* TargetPal = Cast<APWPalBase>(CaptureTarget))
	{
		PendingCaptureChance = CaptureComponent ? CaptureComponent->CalculateCaptureChanceForTarget(TargetPal) : 0.f;
		PendingCaptureRoll = FMath::FRand();
		bPendingCaptureSuccess = PendingCaptureRoll <= PendingCaptureChance;
	}

	CaptureFailureProgress = bPendingCaptureSuccess ? 1.f : CalculateFailureProgressFromRoll();
	CaptureUiTargetChance = bPendingCaptureSuccess ? 1.f : CalculateFailureDisplayChance();

	HitReactStartLocation = Hit.ImpactPoint + Hit.ImpactNormal * 12.f;

	const FVector IncomingDirection = ImpactVelocity.GetSafeNormal();
	const FVector BackDirection = (-IncomingDirection).GetSafeNormal();
	const FVector SafeBackDirection = BackDirection.IsNearlyZero() ? Hit.ImpactNormal.GetSafeNormal() : BackDirection;

	HitReactTargetLocation = HitReactStartLocation
		+ SafeBackDirection * HitReactBackDistance
		+ FVector::UpVector * HitReactUpHeight;

	UE_LOG(LogTemp, Display, TEXT("[PWCapture] Sphere hit react. Projectile=%s Target=%s Chance=%.1f%% Roll=%.1f%% Success=%d DisplayTarget=%.1f%% FailureProgress=%.2f Start=%s TargetLocation=%s UpHeight=%.1f BackDistance=%.1f"),
		*GetName(),
		*GetNameSafe(CaptureTarget),
		PendingCaptureChance * 100.f,
		PendingCaptureRoll * 100.f,
		bPendingCaptureSuccess ? 1 : 0,
		CaptureUiTargetChance * 100.f,
		CaptureFailureProgress,
		*HitReactStartLocation.ToString(),
		*HitReactTargetLocation.ToString(),
		HitReactUpHeight,
		HitReactBackDistance);

	SetActorLocation(HitReactStartLocation);
	EnterState(EPWCaptureSphereProjectileState::HitReact);
	APWPalBase* TargetPal = Cast<APWPalBase>(CaptureTarget);
	CaptureUiTarget = TargetPal;
	CaptureUiInitialChance = PendingCaptureChance;
	MulticastStartHitReact(
		HitReactStartLocation,
		HitReactTargetLocation,
		TargetPal,
		PendingCaptureChance,
		CaptureUiTargetChance,
		CaptureFailureProgress);
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

	if (ProjectileState == EPWCaptureSphereProjectileState::Finished)
	{
		VisualRoot->SetRelativeScale3D(FVector::OneVector);
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

void APWCaptureSphereProjectile::ShowCaptureProgressWidget()
{
	if (bCaptureUiStarted || !CaptureProgressWidget)
	{
		return;
	}

	CaptureProgressWidget->SetVisibility(true);
	bCaptureUiStarted = true;
	UpdateCaptureProgressWidget();
}

void APWCaptureSphereProjectile::UpdateCaptureProgressWidget() const
{
	if (!CaptureProgressWidget || !CaptureProgressWidget->IsVisible())
	{
		return;
	}

	UPWCaptureProgressWidget* ProgressWidget = Cast<UPWCaptureProgressWidget>(CaptureProgressWidget->GetWidget());
	if (!ProgressWidget)
	{
		return;
	}

	const bool bCapturing = ProjectileState == EPWCaptureSphereProjectileState::Capturing;
	ProgressWidget->SetCaptureProgress(CalculateCaptureDisplayChance(), bCapturing);
}

void APWCaptureSphereProjectile::HideCaptureProgressWidget() const
{
	if (CaptureProgressWidget)
	{
		CaptureProgressWidget->SetVisibility(false);
	}
}

float APWCaptureSphereProjectile::CalculateCaptureUiProgress() const
{
	switch (ProjectileState)
	{
	case EPWCaptureSphereProjectileState::HitReact:
	{
		const float Alpha = HitReactDuration > 0.f ? FMath::Clamp(StateElapsedTime / HitReactDuration, 0.f, 1.f) : 1.f;
		return Alpha * 0.10f;
	}
	case EPWCaptureSphereProjectileState::Opening:
	{
		const float Alpha = OpenDuration > 0.f ? FMath::Clamp(StateElapsedTime / OpenDuration, 0.f, 1.f) : 1.f;
		return 0.10f + Alpha * 0.15f;
	}
	case EPWCaptureSphereProjectileState::Closing:
	{
		const float Alpha = CloseDuration > 0.f ? FMath::Clamp(StateElapsedTime / CloseDuration, 0.f, 1.f) : 1.f;
		return 0.25f + Alpha * 0.15f;
	}
	case EPWCaptureSphereProjectileState::Capturing:
	{
		const float Alpha = CaptureShakeDuration > 0.f ? FMath::Clamp(StateElapsedTime / CaptureShakeDuration, 0.f, 1.f) : 1.f;
		return 0.40f + Alpha * 0.60f;
	}
	default:
		return 0.f;
	}
}

float APWCaptureSphereProjectile::CalculateCaptureDisplayChance() const
{
	const float Progress = CalculateCaptureUiProgress();
	const float CaptureOnlyProgress = FMath::Clamp((Progress - 0.40f) / 0.60f, 0.f, 1.f);
	const float PulsePosition = CaptureOnlyProgress * 3.f;
	const float PulseIndex = FMath::FloorToFloat(PulsePosition);
	const float PulseAlpha = FMath::Frac(PulsePosition);
	const float PulseSmoothAlpha = FMath::InterpEaseInOut(0.f, 1.f, PulseAlpha, 2.f);
	const float ChanceRiseAlpha = FMath::Clamp((PulseIndex + PulseSmoothAlpha) / 3.f, 0.f, 1.f);
	return FMath::Lerp(CaptureUiInitialChance, CaptureUiTargetChance, ChanceRiseAlpha);
}

void APWCaptureSphereProjectile::SetHitTargetSuppressed(bool bSuppressed)
{
	if (!HasAuthority())
	{
		return;
	}

	AActor* TargetActor = HitTarget.Get();
	if (!IsValid(TargetActor) || bHitTargetSuppressed == bSuppressed)
	{
		return;
	}

	bHitTargetSuppressed = bSuppressed;
	if (APWPalBase* TargetPal = Cast<APWPalBase>(TargetActor))
	{
		TargetPal->SetCaptureInteractionDisabled(bSuppressed);
	}

	if (!bSuppressed)
	{
		TargetActor->SetActorTransform(HitTargetOriginalTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}

	MulticastSetHitTargetSuppressed(TargetActor, bSuppressed, HitTargetOriginalTransform);
	TargetActor->ForceNetUpdate();

	UE_LOG(LogTemp, Display, TEXT("[PWCapture] Capture target %s. Target=%s Location=%s"),
		bSuppressed ? TEXT("entered sphere") : TEXT("restored"),
		*GetNameSafe(TargetActor),
		*TargetActor->GetActorLocation().ToString());
}

void APWCaptureSphereProjectile::RestoreHitTargetIfNeeded()
{
	if (HasAuthority() && bHitTargetSuppressed && !bCaptureResultResolved)
	{
		SetHitTargetSuppressed(false);
	}
}

void APWCaptureSphereProjectile::ResolveCaptureResult()
{
	APWPalBase* TargetPal = Cast<APWPalBase>(HitTarget.Get());
	const bool bCaptured = TargetPal && bPendingCaptureSuccess;
	bCaptureResultResolved = true;

	UE_LOG(LogTemp, Display, TEXT("[PWCapture] Capture result. Projectile=%s Target=%s Success=%d Chance=%.1f%% Roll=%.1f%%"),
		*GetName(),
		*GetNameSafe(TargetPal),
		bCaptured ? 1 : 0,
		PendingCaptureChance * 100.f,
		PendingCaptureRoll * 100.f);

	if (!bCaptured)
	{
		SetHitTargetSuppressed(false);
		return;
	}

	APWPlayerCharacter* OwnerCharacter = Cast<APWPlayerCharacter>(GetOwner());
	UPWPlayerPalStorageComponent* PalStorageComponent = OwnerCharacter ? OwnerCharacter->GetPalStorageComponent() : nullptr;
	if (!PalStorageComponent || !PalStorageComponent->RegisterCapturedPal(TargetPal))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWCapture] Capture succeeded but pal registration failed. Projectile=%s Owner=%s Target=%s"),
			*GetName(),
			*GetNameSafe(OwnerCharacter),
			*GetNameSafe(TargetPal));

		SetHitTargetSuppressed(false);
		return;
	}

	// 등록이 끝난 야생 팰만 월드에서 제거한다. 실패 시에는 위에서 다시 복구한다.
	TargetPal->Destroy();
	HitTarget = nullptr;
}

bool APWCaptureSphereProjectile::IsCaptureTarget(AActor* OtherActor) const
{
	const APWPalBase* TargetPal = Cast<APWPalBase>(OtherActor);
	return IsValid(TargetPal)
		&& !TargetPal->IsActorBeingDestroyed()
		&& !TargetPal->IsPlayerOwnedPal()
		&& !TargetPal->IsCaptureInteractionDisabled()
		&& !TargetPal->IsHidden()
		&& TargetPal->GetActorEnableCollision();
}

bool APWCaptureSphereProjectile::HasValidHitReactLocations() const
{
	return !HitReactStartLocation.IsNearlyZero() || !HitReactTargetLocation.IsNearlyZero();
}

float APWCaptureSphereProjectile::CalculateFailureProgressFromRoll() const
{
	if (PendingCaptureChance >= 1.f)
	{
		return 1.f;
	}

	// 판정 실패도 한 번에 끝내지 않고, 주사위가 아슬아슬할수록 더 늦게 실패하도록 보인다.
	const float FailRange = FMath::Max(1.f - PendingCaptureChance, KINDA_SMALL_NUMBER);
	const float FailDistance = FMath::Clamp((PendingCaptureRoll - PendingCaptureChance) / FailRange, 0.f, 1.f);
	const float ClosenessToSuccess = 1.f - FailDistance;
	const float CaptureOnlyFailureAlpha = FMath::Lerp(0.35f, 0.92f, ClosenessToSuccess);
	return 0.40f + CaptureOnlyFailureAlpha * 0.60f;
}

float APWCaptureSphereProjectile::CalculateFailureDisplayChance() const
{
	const float CaptureOnlyFailureAlpha = FMath::Clamp((CaptureFailureProgress - 0.40f) / 0.60f, 0.f, 1.f);
	const float NearlyFullChance = 0.99f;
	return FMath::Clamp(
		FMath::Lerp(PendingCaptureChance, NearlyFullChance, CaptureOnlyFailureAlpha),
		PendingCaptureChance,
		0.99f);
}

float APWCaptureSphereProjectile::EvaluateCaptureShakeYaw() const
{
	if (CaptureShakeDuration <= 0.f)
	{
		return 0.f;
	}

	// 일정한 진동 대신 포획 판정처럼 보이는 세 번의 "쿵" 리듬을 만든다.
	const float Progress = FMath::Clamp(StateElapsedTime / CaptureShakeDuration, 0.f, 1.f);
	const float PulsePosition = Progress * 3.f;
	const int32 PulseIndex = FMath::Clamp(FMath::FloorToInt(PulsePosition), 0, 2);
	const float LocalAlpha = FMath::Frac(PulsePosition);

	if (LocalAlpha > 0.42f)
	{
		return 0.f;
	}

	const float PulseAlpha = LocalAlpha / 0.42f;
	const float Envelope = FMath::Sin(PulseAlpha * PI);
	const float Direction = PulseIndex % 2 == 0 ? 1.f : -1.f;
	const float Strength = 0.75f + static_cast<float>(PulseIndex) * 0.18f;
	return Direction * Envelope * CaptureShakeYawAmplitude * Strength;
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
