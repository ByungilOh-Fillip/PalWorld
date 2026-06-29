#include "Resource/PW_HarvestableResourceComponent.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UPW_HarvestableResourceComponent::UPW_HarvestableResourceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPW_HarvestableResourceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() != nullptr && GetOwner()->HasAuthority())
	{
		CurrentHealth = MaxHealth;
	}
}

void UPW_HarvestableResourceComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPW_HarvestableResourceComponent, CurrentHealth);
	DOREPLIFETIME(UPW_HarvestableResourceComponent, bIsDepleted);
}

bool UPW_HarvestableResourceComponent::ApplyHarvestDamage(float DamageAmount, AActor* InstigatorActor)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || bIsDepleted || DamageAmount <= 0.0f)
	{
		return false;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	OnHarvested.Broadcast(InstigatorActor, DamageAmount, CurrentHealth, RewardName, RewardAmount);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Harvested %s: Damage=%.2f CurrentHealth=%.2f Reward=%s x%d"),
		*Owner->GetName(),
		DamageAmount,
		CurrentHealth,
		*RewardName.ToString(),
		RewardAmount);

	if (CurrentHealth <= 0.0f)
	{
		DepleteResource(InstigatorActor);
	}

	Owner->ForceNetUpdate();
	return true;
}

void UPW_HarvestableResourceComponent::SetResourceDefaults(FGameplayTag InRequiredWorkTag, FName InRewardName, int32 InRewardAmount)
{
	RequiredWorkTag = InRequiredWorkTag;
	RewardName = InRewardName;
	RewardAmount = FMath::Max(0, InRewardAmount);
}

void UPW_HarvestableResourceComponent::OnRep_CurrentHealth()
{
}

void UPW_HarvestableResourceComponent::OnRep_IsDepleted()
{
	BroadcastDepletedState();

	if (bIsDepleted)
	{
		OnDepleted.Broadcast(nullptr);
	}
	else
	{
		OnRespawned.Broadcast();
	}
}

void UPW_HarvestableResourceComponent::DepleteResource(AActor* InstigatorActor)
{
	if (bIsDepleted)
	{
		return;
	}

	bIsDepleted = true;
	CurrentHealth = 0.0f;

	BroadcastDepletedState();
	OnDepleted.Broadcast(InstigatorActor);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RespawnTimerHandle);
		World->GetTimerManager().SetTimer(
			RespawnTimerHandle,
			this,
			&UPW_HarvestableResourceComponent::RespawnResource,
			RespawnDelay,
			false);
	}
}

void UPW_HarvestableResourceComponent::RespawnResource()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	bIsDepleted = false;
	CurrentHealth = MaxHealth;

	BroadcastDepletedState();
	OnRespawned.Broadcast();
	Owner->ForceNetUpdate();
}

void UPW_HarvestableResourceComponent::BroadcastDepletedState()
{
	OnDepletedStateChanged.Broadcast(bIsDepleted);
}
