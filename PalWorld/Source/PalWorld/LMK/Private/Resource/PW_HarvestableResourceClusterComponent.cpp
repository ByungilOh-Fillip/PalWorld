#include "Resource/PW_HarvestableResourceClusterComponent.h"

#include "Engine/World.h"
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

	for (float& CurrentHealth : CurrentHealthByInstance)
	{
		CurrentHealth = MaxHealth;
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
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	OnInstanceDamaged.Broadcast(InstanceIndex, InstigatorActor, DamageAmount, RewardName, RewardAmount);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Harvested cluster %s instance %d: Damage=%.2f CurrentHealth=%.2f Reward=%s x%d"),
		*Owner->GetName(),
		InstanceIndex,
		DamageAmount,
		CurrentHealth,
		*RewardName.ToString(),
		RewardAmount);

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
	DepletedInstanceIndices.Remove(InstanceIndex);
	RespawnTimerHandles.Remove(InstanceIndex);

	OnInstanceRespawned.Broadcast(InstanceIndex);
	Owner->ForceNetUpdate();
}

bool UPW_HarvestableResourceClusterComponent::IsValidInstanceIndex(int32 InstanceIndex) const
{
	return CurrentHealthByInstance.IsValidIndex(InstanceIndex);
}
