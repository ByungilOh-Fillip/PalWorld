#include "Base/PW_BaseNavigationComponent.h"

#include "Base/PW_BaseCampActor.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "NavigationInvokerComponent.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

UPW_BaseNavigationComponent::UPW_BaseNavigationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UPW_BaseNavigationComponent::ConfigureNavigationInvoker(float CampRadius)
{
	UNavigationInvokerComponent* NavigationInvoker = FindNavigationInvokerComponent();
	if (NavigationInvoker == nullptr)
	{
		return;
	}

	const float GenerationRadius = FMath::Max(0.0f, CampRadius);
	const float RemovalRadius = GenerationRadius + NavRemovalExtraRadius;
	NavigationInvoker->SetGenerationRadii(GenerationRadius, RemovalRadius);
}

void UPW_BaseNavigationComponent::SetBaseNavigationActive(bool bNewActive)
{
	bIsBaseNavigationActive = bNewActive;

	UNavigationInvokerComponent* NavigationInvoker = FindNavigationInvokerComponent();
	if (NavigationInvoker == nullptr)
	{
		return;
	}

	if (bIsBaseNavigationActive)
	{
		NavigationInvoker->Activate();
	}
	else
	{
		NavigationInvoker->Deactivate();
	}
}

bool UPW_BaseNavigationComponent::ProjectPointToBaseNavigation(const FVector& Location, FVector& OutProjectedLocation) const
{
	if (!bIsBaseNavigationActive)
	{
		return false;
	}

	UWorld* World = GetWorld();
	UNavigationSystemV1* NavigationSystem = World != nullptr ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	if (NavigationSystem == nullptr)
	{
		return false;
	}

	FNavLocation NavLocation;
	if (!NavigationSystem->ProjectPointToNavigation(Location, NavLocation, NavigationProjectionExtent))
	{
		return false;
	}

	OutProjectedLocation = NavLocation.Location;
	return true;
}

bool UPW_BaseNavigationComponent::FindPathToLocation(const FVector& StartLocation, const FVector& EndLocation) const
{
	if (!bIsBaseNavigationActive)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	UNavigationPath* NavigationPath = UNavigationSystemV1::FindPathToLocationSynchronously(World, StartLocation, EndLocation, GetOwner());
	return NavigationPath != nullptr && NavigationPath->IsValid() && !NavigationPath->IsPartial();
}

bool UPW_BaseNavigationComponent::GetRandomReachablePointInBase(const FVector& Origin, float Radius, FVector& OutLocation) const
{
	if (!bIsBaseNavigationActive)
	{
		return false;
	}

	UWorld* World = GetWorld();
	UNavigationSystemV1* NavigationSystem = World != nullptr ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	if (NavigationSystem == nullptr)
	{
		return false;
	}

	const float BaseRadius = GetBaseCampRadiusFallback();
	const float QueryRadius = FMath::Clamp(Radius, 0.0f, BaseRadius);
	FNavLocation NavLocation;
	if (!NavigationSystem->GetRandomReachablePointInRadius(Origin, QueryRadius, NavLocation))
	{
		return false;
	}

	const AActor* Owner = GetOwner();
	if (Owner != nullptr && FVector::DistSquared2D(Owner->GetActorLocation(), NavLocation.Location) > FMath::Square(BaseRadius))
	{
		return false;
	}

	OutLocation = NavLocation.Location;
	return true;
}

bool UPW_BaseNavigationComponent::IsNavigationReadyForBase() const
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}

	FVector ProjectedLocation;
	return ProjectPointToBaseNavigation(Owner->GetActorLocation(), ProjectedLocation);
}

void UPW_BaseNavigationComponent::DrawDebugNavigationSample() const
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!bDrawDebugNavigation)
	{
		return;
	}

	UWorld* World = GetWorld();
	const AActor* Owner = GetOwner();
	if (World == nullptr || Owner == nullptr || World->IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	FVector ProjectedLocation;
	if (ProjectPointToBaseNavigation(Owner->GetActorLocation(), ProjectedLocation))
	{
		DrawDebugSphere(World, ProjectedLocation, 80.0f, 16, FColor::Cyan, false, 10.0f, 0, 4.0f);
		DrawDebugLine(World, Owner->GetActorLocation(), ProjectedLocation, FColor::Cyan, false, 10.0f, 0, 3.0f);
	}
#endif
}

UNavigationInvokerComponent* UPW_BaseNavigationComponent::FindNavigationInvokerComponent() const
{
	const AActor* Owner = GetOwner();
	return Owner != nullptr ? Owner->FindComponentByClass<UNavigationInvokerComponent>() : nullptr;
}

float UPW_BaseNavigationComponent::GetBaseCampRadiusFallback() const
{
	const APW_BaseCampActor* BaseCampActor = Cast<APW_BaseCampActor>(GetOwner());
	return BaseCampActor != nullptr ? BaseCampActor->GetCampRadius() : 1000.0f;
}
