#include "Resource/PW_HarvestDebugTraceComponent.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/PW_HarvestDamageTarget.h"
#include "Interfaces/PW_HarvestInstanceDamageTarget.h"

UPW_HarvestDebugTraceComponent::UPW_HarvestDebugTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPW_HarvestDebugTraceComponent::TryHarvestForward()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	FHitResult HitResult;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantizeNormal TraceDirection;
	if (!TraceHarvestTarget(HitResult, TraceStart, TraceDirection))
	{
		return;
	}

	if (!Owner->HasAuthority())
	{
		ServerTryHarvestForward(TraceStart, TraceDirection);
		return;
	}

	ApplyHarvestDamageToHit(HitResult);
}

void UPW_HarvestDebugTraceComponent::ServerTryHarvestForward_Implementation(FVector_NetQuantize TraceStart, FVector_NetQuantizeNormal TraceDirection)
{
	FHitResult HitResult;
	if (TraceHarvestTargetFromView(HitResult, TraceStart, FVector(TraceDirection)))
	{
		ApplyHarvestDamageToHit(HitResult);
	}
}

bool UPW_HarvestDebugTraceComponent::TraceHarvestTarget(
	FHitResult& OutHit,
	FVector_NetQuantize& OutTraceStart,
	FVector_NetQuantizeNormal& OutTraceDirection) const
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}

	FVector ViewLocation = Owner->GetActorLocation();
	FRotator ViewRotation = Owner->GetActorRotation();

	if (const AController* Controller = GetOwningController())
	{
		Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}

	OutTraceStart = FVector_NetQuantize(ViewLocation);
	OutTraceDirection = FVector_NetQuantizeNormal(ViewRotation.Vector());

	return TraceHarvestTargetFromView(OutHit, FVector(OutTraceStart), FVector(OutTraceDirection));
}

bool UPW_HarvestDebugTraceComponent::TraceHarvestTargetFromView(
	FHitResult& OutHit,
	const FVector& TraceStart,
	const FVector& TraceDirection) const
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}

	const FVector SafeDirection = TraceDirection.GetSafeNormal();
	if (SafeDirection.IsNearlyZero())
	{
		return false;
	}

	const FVector TraceEnd = TraceStart + SafeDirection * TraceDistance;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PW_HarvestDebugTrace), false, Owner);
	const bool bHit = GetWorld() != nullptr
		&& GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, TraceChannel, QueryParams);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	const FColor DebugColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, DebugColor, false, 1.0f, 0, 1.5f);
#endif

	return bHit;
}

bool UPW_HarvestDebugTraceComponent::ApplyHarvestDamageToHit(const FHitResult& HitResult) const
{
	AActor* HitActor = HitResult.GetActor();
	if (HitActor == nullptr)
	{
		return false;
	}

	if (HitResult.Item != INDEX_NONE && HitActor->GetClass()->ImplementsInterface(UPW_HarvestInstanceDamageTarget::StaticClass()))
	{
		return IPW_HarvestInstanceDamageTarget::Execute_ApplyHarvestDamageToInstance(
			HitActor,
			HitResult.Item,
			HarvestDamage,
			GetOwner());
	}

	if (!HitActor->GetClass()->ImplementsInterface(UPW_HarvestDamageTarget::StaticClass()))
	{
		return false;
	}

	return IPW_HarvestDamageTarget::Execute_ApplyHarvestDamage(HitActor, HarvestDamage, GetOwner());
}

AController* UPW_HarvestDebugTraceComponent::GetOwningController() const
{
	const AActor* Owner = GetOwner();
	if (const APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		return OwnerPawn->GetController();
	}

	return Owner != nullptr ? Owner->GetInstigatorController() : nullptr;
}
