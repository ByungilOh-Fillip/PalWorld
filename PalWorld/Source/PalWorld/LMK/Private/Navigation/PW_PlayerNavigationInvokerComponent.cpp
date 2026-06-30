#include "Navigation/PW_PlayerNavigationInvokerComponent.h"

#include "GameFramework/Actor.h"
#include "NavigationInvokerComponent.h"

UPW_PlayerNavigationInvokerComponent::UPW_PlayerNavigationInvokerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UPW_PlayerNavigationInvokerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoConfigureOnBeginPlay)
	{
		ConfigureNavigationInvoker();
	}

	if (bAutoActivateNavigationInvoker)
	{
		SetPlayerNavigationInvokerActive(true);
	}
}

void UPW_PlayerNavigationInvokerComponent::ConfigureNavigationInvoker()
{
	UNavigationInvokerComponent* NavigationInvoker = FindNavigationInvokerComponent();
	if (NavigationInvoker == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWNavigation] %s needs a NavigationInvokerComponent on the same Actor."), *GetNameSafe(GetOwner()));
		return;
	}

	const float ResolvedGenerationRadius = GetResolvedGenerationRadius();
	const float ResolvedRemovalRadius = FMath::Max(RemovalRadius, ResolvedGenerationRadius);
	NavigationInvoker->SetGenerationRadii(ResolvedGenerationRadius, ResolvedRemovalRadius);
}

void UPW_PlayerNavigationInvokerComponent::SetPlayerNavigationInvokerActive(bool bActive)
{
	UNavigationInvokerComponent* NavigationInvoker = FindNavigationInvokerComponent();
	if (NavigationInvoker == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWNavigation] %s cannot toggle navigation without a NavigationInvokerComponent."), *GetNameSafe(GetOwner()));
		return;
	}

	if (bActive)
	{
		NavigationInvoker->Activate();
	}
	else
	{
		NavigationInvoker->Deactivate();
	}
}

bool UPW_PlayerNavigationInvokerComponent::IsPlayerNavigationInvokerReady() const
{
	return FindNavigationInvokerComponent() != nullptr;
}

float UPW_PlayerNavigationInvokerComponent::GetResolvedGenerationRadius() const
{
	float ResolvedRadius = GenerationRadius;
	const AActor* Owner = GetOwner();
	if (bUseOwnerNetCullDistanceAsFallback && Owner != nullptr && Owner->NetCullDistanceSquared > 0.0f)
	{
		ResolvedRadius = FMath::Sqrt(Owner->NetCullDistanceSquared);
	}

	return FMath::Clamp(ResolvedRadius, MinGenerationRadius, MaxGenerationRadius);
}

float UPW_PlayerNavigationInvokerComponent::GetRecommendedGenerationRadiusForWildPalSpawner(float ActivationDistance, float SpawnRadius)
{
	return FMath::Max(0.0f, ActivationDistance) + FMath::Max(0.0f, SpawnRadius);
}

void UPW_PlayerNavigationInvokerComponent::ApplyWildPalSpawnerCompatibleDefaults()
{
	GenerationRadius = GetRecommendedGenerationRadiusForWildPalSpawner(8000.0f, 3000.0f);
	RemovalRadius = GenerationRadius + 2000.0f;
	MinGenerationRadius = 8000.0f;
	MaxGenerationRadius = 15000.0f;
	bUseOwnerNetCullDistanceAsFallback = false;
	ConfigureNavigationInvoker();
}

UNavigationInvokerComponent* UPW_PlayerNavigationInvokerComponent::FindNavigationInvokerComponent() const
{
	const AActor* Owner = GetOwner();
	return Owner != nullptr ? Owner->FindComponentByClass<UNavigationInvokerComponent>() : nullptr;
}
