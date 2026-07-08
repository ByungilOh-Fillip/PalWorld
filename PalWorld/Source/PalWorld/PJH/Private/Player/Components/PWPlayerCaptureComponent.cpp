// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerCaptureComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Player/Components/PWPlayerInventoryLinkComponent.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "PWPalBase.h"
#include "StatusComponent.h"
#include "World/PWCaptureSphereProjectile.h"

UPWPlayerCaptureComponent::UPWPlayerCaptureComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UPWPlayerCaptureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateCaptureAimInfo();
}

bool UPWPlayerCaptureComponent::HasThrowableCaptureSphere() const
{
	const APWPlayerCharacter* PlayerCharacter = GetOwnerCharacter();
	const UPWPlayerInventoryLinkComponent* InventoryLinkComponent = PlayerCharacter ? PlayerCharacter->GetInventoryLinkComponent() : nullptr;
	return InventoryLinkComponent
		&& !CaptureSphereItemId.IsNone()
		&& CaptureSphereCost > 0
		&& InventoryLinkComponent->GetItemCount(CaptureSphereItemId) >= CaptureSphereCost;
}

FText UPWPlayerCaptureComponent::GetCaptureAimTargetNameText() const
{
	const APWPalBase* TargetPal = CaptureAimTarget.Get();
	if (!TargetPal)
	{
		return FText::GetEmpty();
	}

	return FText::FromString(TargetPal->GetName());
}

int32 UPWPlayerCaptureComponent::GetThrowableCaptureSphereCount() const
{
	const APWPlayerCharacter* PlayerCharacter = GetOwnerCharacter();
	const UPWPlayerInventoryLinkComponent* InventoryLinkComponent = PlayerCharacter ? PlayerCharacter->GetInventoryLinkComponent() : nullptr;
	return InventoryLinkComponent && !CaptureSphereItemId.IsNone() ? InventoryLinkComponent->GetItemCount(CaptureSphereItemId) : 0;
}

bool UPWPlayerCaptureComponent::TryThrowCaptureSphere(const FVector& ThrowDirection)
{
	APWPlayerCharacter* PlayerCharacter = GetOwnerCharacter();
	if (!PlayerCharacter)
	{
		return false;
	}

	FVector SafeThrowDirection = ThrowDirection.GetSafeNormal();
	if (SafeThrowDirection.IsNearlyZero())
	{
		SafeThrowDirection = PlayerCharacter->GetActorForwardVector();
	}

	if (!PlayerCharacter->HasAuthority())
	{
		ServerThrowCaptureSphere(FVector_NetQuantizeNormal(SafeThrowDirection));
		return true;
	}

	ThrowCaptureSphere_Server(SafeThrowDirection);
	return true;
}

APWPlayerCharacter* UPWPlayerCaptureComponent::GetOwnerCharacter() const
{
	return Cast<APWPlayerCharacter>(GetOwner());
}

bool UPWPlayerCaptureComponent::IsValidCaptureAimTarget(APWPalBase* TargetPal) const
{
	return IsValid(TargetPal)
		&& !TargetPal->IsActorBeingDestroyed()
		&& !TargetPal->IsCaptureInteractionDisabled()
		&& !TargetPal->IsHidden()
		&& TargetPal->GetActorEnableCollision();
}

void UPWPlayerCaptureComponent::UpdateCaptureAimInfo()
{
	APWPlayerCharacter* PlayerCharacter = GetOwnerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->IsLocallyControlled() || !PlayerCharacter->IsSphereAiming())
	{
		ClearCaptureAimInfo();
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(PlayerCharacter->GetController());
	if (!PlayerController || !PlayerController->PlayerCameraManager)
	{
		ClearCaptureAimInfo();
		return;
	}

	const FVector TraceStart = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FVector TraceDirection = PlayerController->PlayerCameraManager->GetCameraRotation().Vector();
	const FVector TraceEnd = TraceStart + TraceDirection * CaptureAimTraceDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWCaptureAimTrace), false, PlayerCharacter);
	QueryParams.AddIgnoredActor(PlayerCharacter);

	FHitResult Hit;
	const bool bHit = GetWorld() && GetWorld()->SweepSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(CaptureAimTraceRadius),
		QueryParams);

	APWPalBase* TargetPal = bHit ? Cast<APWPalBase>(Hit.GetActor()) : nullptr;
	if (!IsValidCaptureAimTarget(TargetPal))
	{
		TargetPal = nullptr;
	}

	const float Chance = TargetPal ? CalculateCaptureChanceForTarget(TargetPal) : 0.f;
	SetCaptureAimInfo(TargetPal != nullptr, TargetPal, Chance);
}

void UPWPlayerCaptureComponent::ClearCaptureAimInfo()
{
	SetCaptureAimInfo(false, nullptr, 0.f);
}

void UPWPlayerCaptureComponent::SetCaptureAimInfo(bool bNewVisible, APWPalBase* NewTarget, float NewChance)
{
	const bool bNewHasTarget = NewTarget != nullptr;
	const float ClampedChance = bNewHasTarget ? FMath::Clamp(NewChance, 0.f, 1.f) : 0.f;

	if (bCaptureAimVisible == bNewVisible
		&& bHasCaptureAimTarget == bNewHasTarget
		&& CaptureAimTarget.Get() == NewTarget
		&& FMath::IsNearlyEqual(CaptureAimChance, ClampedChance, 0.001f))
	{
		return;
	}

	bCaptureAimVisible = bNewVisible;
	bHasCaptureAimTarget = bNewHasTarget;
	CaptureAimTarget = NewTarget;
	CaptureAimChance = ClampedChance;

	OnCaptureAimInfoChanged.Broadcast();
}

float UPWPlayerCaptureComponent::CalculateCaptureChanceForTarget(APWPalBase* TargetPal) const
{
	if (!TargetPal)
	{
		return 0.f;
	}

	const UStatusComponent* TargetStatus = TargetPal->FindComponentByClass<UStatusComponent>();
	const float HealthRatio = TargetStatus && TargetStatus->MaxHP > 0.f
		? FMath::Clamp(TargetStatus->CurrentHP / TargetStatus->MaxHP, 0.f, 1.f)
		: 1.f;

	const float MissingHealthRatio = 1.f - HealthRatio;
	const float Chance = BaseCaptureChance + MissingHealthRatio * MissingHealthCaptureChanceBonus;
	return FMath::Clamp(Chance, MinimumCaptureChance, MaximumCaptureChance);
}

void UPWPlayerCaptureComponent::ThrowCaptureSphere_Server(const FVector& ThrowDirection)
{
	APWPlayerCharacter* PlayerCharacter = GetOwnerCharacter();
	if (!PlayerCharacter)
	{
		return;
	}

	UPWPlayerInventoryLinkComponent* InventoryLinkComponent = PlayerCharacter->GetInventoryLinkComponent();
	if (!InventoryLinkComponent || !InventoryLinkComponent->ConsumeItem(CaptureSphereItemId, CaptureSphereCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWCapture] Throw sphere rejected. No sphere item. Player=%s ItemId=%s Cost=%d"),
			*PlayerCharacter->GetName(),
			*CaptureSphereItemId.ToString(),
			CaptureSphereCost);
		return;
	}

	UWorld* World = PlayerCharacter->GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SafeThrowDirection = ThrowDirection.GetSafeNormal();
	const FVector SpawnDirection = SafeThrowDirection.IsNearlyZero() ? PlayerCharacter->GetActorForwardVector() : SafeThrowDirection;
	const FVector SpawnLocation = PlayerCharacter->GetActorLocation()
		+ SpawnDirection * ProjectileSpawnForwardOffset
		+ FVector(0.f, 0.f, ProjectileSpawnUpOffset);
	const FRotator SpawnRotation = SpawnDirection.Rotation();
	TSubclassOf<APWCaptureSphereProjectile> ProjectileClass = CaptureSphereProjectileClass;
	if (!ProjectileClass)
	{
		ProjectileClass = APWCaptureSphereProjectile::StaticClass();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = PlayerCharacter;
	SpawnParameters.Instigator = PlayerCharacter;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APWCaptureSphereProjectile* Projectile = World->SpawnActor<APWCaptureSphereProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);

	if (Projectile)
	{
		Projectile->LaunchInDirection(SpawnDirection);
	}

	UE_LOG(LogTemp, Display, TEXT("[PWCapture] Throw sphere requested. Player=%s ItemId=%s Direction=%s Location=%s Projectile=%s"),
		*PlayerCharacter->GetName(),
		*CaptureSphereItemId.ToString(),
		*SpawnDirection.ToString(),
		*SpawnLocation.ToString(),
		*GetNameSafe(Projectile));
}

void UPWPlayerCaptureComponent::ServerThrowCaptureSphere_Implementation(FVector_NetQuantizeNormal ThrowDirection)
{
	ThrowCaptureSphere_Server(ThrowDirection);
}
