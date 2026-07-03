#include "Resource/PW_HarvestableResourceClusterComponent.h"

#include "Engine/World.h"
#include "GameplayTags/PW_GameplayTags.h"
#include "Interfaces/PW_ItemReceiver.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UPW_HarvestableResourceClusterComponent::UPW_HarvestableResourceClusterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPW_HarvestableResourceClusterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPW_HarvestableResourceClusterComponent, DepletedInstanceIndices);
}

void UPW_HarvestableResourceClusterComponent::InitializeInstances(int32 InstanceCount)
{
	const int32 SafeInstanceCount = FMath::Max(0, InstanceCount);
	CurrentHealthByInstance.SetNum(SafeInstanceCount);
	RewardDamageProgressByInstance.SetNum(SafeInstanceCount);

	for (float& CurrentHealth : CurrentHealthByInstance)
	{
		CurrentHealth = MaxHealth;
	}

	for (float& RewardDamageProgress : RewardDamageProgressByInstance)
	{
		RewardDamageProgress = 0.f;
	}

	for (int32 Index = DepletedInstanceIndices.Num() - 1; Index >= 0; --Index)
	{
		if (!IsValidInstanceIndex(DepletedInstanceIndices[Index]))
		{
			DepletedInstanceIndices.RemoveAtSwap(Index);
		}
	}
}

bool UPW_HarvestableResourceClusterComponent::ApplyHarvestDamageToInstance(
	int32 InstanceIndex,
	float DamageAmount,
	AActor* InstigatorActor)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || !IsValidInstanceIndex(InstanceIndex) || DamageAmount <= 0.0f)
	{
		return false;
	}

	if (IsInstanceDepleted(InstanceIndex))
	{
		return false;
	}

	float& CurrentHealth = CurrentHealthByInstance[InstanceIndex];
	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	const float AppliedDamage = PreviousHealth - CurrentHealth;
	const int32 RewardMultiplier = ConsumeRewardIntervals(InstanceIndex, AppliedDamage);
	const int32 GrantedRewardAmount = RewardMultiplier * RewardAmount;
	const FName GrantedRewardName = ResolveRewardName();

	GrantReward(InstigatorActor, RewardMultiplier);
	OnInstanceDamaged.Broadcast(InstanceIndex, InstigatorActor, AppliedDamage, GrantedRewardName, GrantedRewardAmount);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Harvested cluster %s instance %d: Damage=%.2f CurrentHealth=%.2f Reward=%s x%d"),
		*Owner->GetName(),
		InstanceIndex,
		AppliedDamage,
		CurrentHealth,
		*GrantedRewardName.ToString(),
		GrantedRewardAmount);

	if (CurrentHealth <= 0.0f)
	{
		DepleteInstance(InstanceIndex, InstigatorActor);
	}

	Owner->ForceNetUpdate();
	return true;
}

bool UPW_HarvestableResourceClusterComponent::IsInstanceDepleted(int32 InstanceIndex) const
{
	return DepletedInstanceIndices.Contains(InstanceIndex);
}

void UPW_HarvestableResourceClusterComponent::OnRep_DepletedInstanceIndices()
{
	OnReplicatedStateChanged.Broadcast();
}

void UPW_HarvestableResourceClusterComponent::DepleteInstance(int32 InstanceIndex, AActor* InstigatorActor)
{
	if (!IsValidInstanceIndex(InstanceIndex) || IsInstanceDepleted(InstanceIndex))
	{
		return;
	}

	CurrentHealthByInstance[InstanceIndex] = 0.0f;
	RewardDamageProgressByInstance[InstanceIndex] = 0.f;
	DepletedInstanceIndices.Add(InstanceIndex);
	OnInstanceDepleted.Broadcast(InstanceIndex, InstigatorActor);

	if (UWorld* World = GetWorld())
	{
		FTimerHandle& RespawnTimerHandle = RespawnTimerHandles.FindOrAdd(InstanceIndex);
		World->GetTimerManager().ClearTimer(RespawnTimerHandle);
		World->GetTimerManager().SetTimer(
			RespawnTimerHandle,
			FTimerDelegate::CreateUObject(this, &UPW_HarvestableResourceClusterComponent::RespawnInstance, InstanceIndex),
			RespawnDelay,
			false);
	}
}

void UPW_HarvestableResourceClusterComponent::RespawnInstance(int32 InstanceIndex)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || !IsValidInstanceIndex(InstanceIndex))
	{
		return;
	}

	CurrentHealthByInstance[InstanceIndex] = MaxHealth;
	RewardDamageProgressByInstance[InstanceIndex] = 0.f;
	DepletedInstanceIndices.Remove(InstanceIndex);
	RespawnTimerHandles.Remove(InstanceIndex);

	OnInstanceRespawned.Broadcast(InstanceIndex);
	Owner->ForceNetUpdate();
}

bool UPW_HarvestableResourceClusterComponent::IsValidInstanceIndex(int32 InstanceIndex) const
{
	return CurrentHealthByInstance.IsValidIndex(InstanceIndex);
}

FName UPW_HarvestableResourceClusterComponent::ResolveRewardName() const
{
	if (!RewardName.IsNone())
	{
		const FString RewardString = RewardName.ToString();
		if (RewardString.Contains(TEXT("Rock")) || RewardString.Contains(TEXT("Ore")))
		{
			return TEXT("Stone");
		}

		return RewardName;
	}

	if (RequiredWorkTag.MatchesTagExact(PW_GameplayTags::Work_Mining))
	{
		return TEXT("Stone");
	}

	if (RequiredWorkTag.MatchesTagExact(PW_GameplayTags::Work_Lumbering))
	{
		return TEXT("Wood");
	}

	return NAME_None;
}

int32 UPW_HarvestableResourceClusterComponent::ConsumeRewardIntervals(int32 InstanceIndex, float AppliedDamage)
{
	const FName GrantedRewardName = ResolveRewardName();
	if (!RewardDamageProgressByInstance.IsValidIndex(InstanceIndex)
		|| AppliedDamage <= 0.f
		|| RewardAmount <= 0
		|| GrantedRewardName.IsNone())
	{
		return 0;
	}

	if (RewardDamageInterval <= 0.f)
	{
		return 1;
	}

	float& RewardDamageProgress = RewardDamageProgressByInstance[InstanceIndex];
	RewardDamageProgress += AppliedDamage;

	const int32 RewardMultiplier = FMath::FloorToInt(RewardDamageProgress / RewardDamageInterval);
	if (RewardMultiplier > 0)
	{
		RewardDamageProgress = FMath::Fmod(RewardDamageProgress, RewardDamageInterval);
	}

	return RewardMultiplier;
}

void UPW_HarvestableResourceClusterComponent::GrantReward(AActor* InstigatorActor, int32 RewardMultiplier) const
{
	const int32 GrantedRewardAmount = RewardMultiplier * RewardAmount;
	const FName GrantedRewardName = ResolveRewardName();
	if (!InstigatorActor || GrantedRewardAmount <= 0 || GrantedRewardName.IsNone())
	{
		return;
	}

	if (InstigatorActor->GetClass()->ImplementsInterface(UPW_ItemReceiver::StaticClass()))
	{
		IPW_ItemReceiver::Execute_ReceiveItem(InstigatorActor, GrantedRewardName, GrantedRewardAmount);
	}
}
