#include "Resource/PW_HarvestableResourceComponent.h"

#include "Engine/World.h"
#include "Interfaces/PW_ItemReceiver.h"
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
		RewardDamageProgress = 0.f;
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

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	const float AppliedDamage = PreviousHealth - CurrentHealth;
	const int32 RewardMultiplier = ConsumeRewardIntervals(AppliedDamage);
	const int32 GrantedRewardAmount = RewardMultiplier * RewardAmount;

	GrantReward(InstigatorActor, RewardMultiplier);
	OnHarvested.Broadcast(InstigatorActor, AppliedDamage, CurrentHealth, RewardName, GrantedRewardAmount);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Harvested %s: Damage=%.2f CurrentHealth=%.2f Reward=%s x%d"),
		*Owner->GetName(),
		AppliedDamage,
		CurrentHealth,
		*RewardName.ToString(),
		GrantedRewardAmount);

	if (CurrentHealth <= 0.0f)
	{
		DepleteResource(InstigatorActor);
	}

	Owner->ForceNetUpdate();
	return true;
}

void UPW_HarvestableResourceComponent::SetResourceDefaults(
	FGameplayTag InRequiredWorkTag,
	FName InRewardName,
	int32 InRewardAmount,
	float InRewardDamageInterval)
{
	RequiredWorkTag = InRequiredWorkTag;
	RewardName = InRewardName;
	RewardAmount = FMath::Max(0, InRewardAmount);
	RewardDamageInterval = FMath::Max(0.f, InRewardDamageInterval);
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
	RewardDamageProgress = 0.f;

	BroadcastDepletedState();
	OnRespawned.Broadcast();
	Owner->ForceNetUpdate();
}

void UPW_HarvestableResourceComponent::BroadcastDepletedState()
{
	OnDepletedStateChanged.Broadcast(bIsDepleted);
}

int32 UPW_HarvestableResourceComponent::ConsumeRewardIntervals(float AppliedDamage)
{
	if (AppliedDamage <= 0.f || RewardAmount <= 0 || RewardName.IsNone())
	{
		return 0;
	}

	if (RewardDamageInterval <= 0.f)
	{
		return 1;
	}

	RewardDamageProgress += AppliedDamage;
	const int32 RewardMultiplier = FMath::FloorToInt(RewardDamageProgress / RewardDamageInterval);
	if (RewardMultiplier > 0)
	{
		RewardDamageProgress = FMath::Fmod(RewardDamageProgress, RewardDamageInterval);
	}

	return RewardMultiplier;
}

void UPW_HarvestableResourceComponent::GrantReward(AActor* InstigatorActor, int32 RewardMultiplier) const
{
	const int32 GrantedRewardAmount = RewardMultiplier * RewardAmount;
	if (!InstigatorActor || GrantedRewardAmount <= 0 || RewardName.IsNone())
	{
		return;
	}

	if (InstigatorActor->GetClass()->ImplementsInterface(UPW_ItemReceiver::StaticClass()))
	{
		IPW_ItemReceiver::Execute_ReceiveItem(InstigatorActor, RewardName, GrantedRewardAmount);
	}
}
