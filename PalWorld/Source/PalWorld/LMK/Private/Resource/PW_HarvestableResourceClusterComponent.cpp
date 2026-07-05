#include "Resource/PW_HarvestableResourceClusterComponent.h"

#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "PW_GameplayTags.h"
#include "Interfaces/PW_ItemReceiver.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPlayerInventoryLinkComponent.h"
#include "Player/Data/PWItemDataAsset.h"
#include "Resource/PW_HarvestableResourceCluster.h"
#include "TimerManager.h"
#include "World/PWWorldItemDropLibrary.h"
#include "World/PWWorldItemActor.h"

UPW_HarvestableResourceClusterComponent::UPW_HarvestableResourceClusterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	WorldItemActorClass = APWWorldItemActor::StaticClass();
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

	GrantReward(InstigatorActor, RewardMultiplier, InstanceIndex);
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

AActor* UPW_HarvestableResourceClusterComponent::ResolveRewardReceiver(AActor* InstigatorActor) const
{
	for (AActor* Candidate = InstigatorActor; Candidate != nullptr; Candidate = Candidate->GetOwner())
	{
		if (Candidate->GetClass()->ImplementsInterface(UPW_ItemReceiver::StaticClass()))
		{
			return Candidate;
		}
	}

	return nullptr;
}

UPWItemDataAsset* UPW_HarvestableResourceClusterComponent::ResolveRewardItemData(AActor* RewardReceiver, FName GrantedRewardName) const
{
	const UPWPlayerInventoryLinkComponent* InventoryComponent = RewardReceiver ? RewardReceiver->FindComponentByClass<UPWPlayerInventoryLinkComponent>() : nullptr;
	return InventoryComponent ? InventoryComponent->GetItemDefinition(GrantedRewardName) : nullptr;
}

FVector UPW_HarvestableResourceClusterComponent::GetRewardDropLocation(int32 InstanceIndex) const
{
	const AActor* Owner = GetOwner();
	const APW_HarvestableResourceCluster* ClusterOwner = Cast<APW_HarvestableResourceCluster>(Owner);

	FTransform InstanceTransform;
	if (ClusterOwner && ClusterOwner->GetInstanceWorldTransform(InstanceIndex, InstanceTransform))
	{
		return InstanceTransform.GetLocation() + DropLocationOffset;
	}

	return Owner ? Owner->GetActorLocation() + DropLocationOffset : DropLocationOffset;
}

void UPW_HarvestableResourceClusterComponent::GrantRewardDirectDelayed(AActor* RewardReceiver, FName GrantedRewardName, int32 GrantedRewardAmount)
{
	if (!RewardReceiver || GrantedRewardName.IsNone() || GrantedRewardAmount <= 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	const float MinDelay = FMath::Max(0.f, AutoCollectMinDelay);
	const float MaxDelay = FMath::Max(MinDelay, AutoCollectMaxDelay);
	const float CollectDelay = MaxDelay > 0.f ? FMath::FRandRange(MinDelay, MaxDelay) : 0.f;
	TWeakObjectPtr<AActor> WeakRewardReceiver = RewardReceiver;

	auto GrantRewardNow = [WeakRewardReceiver, GrantedRewardName, GrantedRewardAmount]()
	{
		AActor* Receiver = WeakRewardReceiver.Get();
		if (Receiver && Receiver->GetClass()->ImplementsInterface(UPW_ItemReceiver::StaticClass()))
		{
			IPW_ItemReceiver::Execute_ReceiveItem(Receiver, GrantedRewardName, GrantedRewardAmount);
		}
	};

	if (!World || CollectDelay <= 0.f)
	{
		GrantRewardNow();
		return;
	}

	FTimerHandle AutoCollectTimerHandle;
	World->GetTimerManager().SetTimer(
		AutoCollectTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, MoveTemp(GrantRewardNow)),
		CollectDelay,
		false);
}

void UPW_HarvestableResourceClusterComponent::GrantReward(AActor* InstigatorActor, int32 RewardMultiplier, int32 InstanceIndex)
{
	const int32 GrantedRewardAmount = RewardMultiplier * RewardAmount;
	const FName GrantedRewardName = ResolveRewardName();
	if (!InstigatorActor || GrantedRewardAmount <= 0 || GrantedRewardName.IsNone())
	{
		return;
	}

	AActor* RewardReceiver = ResolveRewardReceiver(InstigatorActor);
	if (!RewardReceiver)
	{
		UE_LOG(LogTemp, Warning, TEXT("Harvest cluster reward has no receiver. Instigator=%s Reward=%s x%d"),
			*InstigatorActor->GetName(),
			*GrantedRewardName.ToString(),
			GrantedRewardAmount);
		return;
	}

	UWorld* World = GetWorld();
	if (!bSpawnWorldDrop || !World)
	{
		UE_LOG(LogTemp, Warning, TEXT("Harvest cluster reward uses direct collect fallback. SpawnWorldDrop=%s World=%s"),
			bSpawnWorldDrop ? TEXT("true") : TEXT("false"),
			World ? TEXT("valid") : TEXT("none"));
		GrantRewardDirectDelayed(RewardReceiver, GrantedRewardName, GrantedRewardAmount);
		return;
	}

	const FVector ResourceDropLocation = GetRewardDropLocation(InstanceIndex);
	UPWItemDataAsset* RewardItemData = ResolveRewardItemData(RewardReceiver, GrantedRewardName);
	if (!UPWWorldItemDropLibrary::CanSpawnWorldItem(RewardItemData))
	{
		UE_LOG(LogTemp, Warning, TEXT("Harvest cluster reward has no valid world item data. Reward=%s ItemData=%s WorldMesh=%s. Use direct collect fallback."),
			*GrantedRewardName.ToString(),
			*GetNameSafe(RewardItemData),
			RewardItemData && RewardItemData->GetWorldMesh() ? *RewardItemData->GetWorldMesh()->GetName() : TEXT("None"));
		GrantRewardDirectDelayed(RewardReceiver, GrantedRewardName, GrantedRewardAmount);
		return;
	}

	FPWWorldItemDropRequest DropRequest;
	DropRequest.ItemData = RewardItemData;
	DropRequest.ItemId = GrantedRewardName;
	DropRequest.Count = GrantedRewardAmount;
	DropRequest.PreferredReceiver = RewardReceiver;
	DropRequest.SourceActor = GetOwner();
	DropRequest.SourceLocation = ResourceDropLocation;
	DropRequest.TargetActor = InstigatorActor;
	DropRequest.TowardTargetMinAlpha = DropTowardInstigatorMinAlpha;
	DropRequest.TowardTargetMaxAlpha = DropTowardInstigatorMaxAlpha;
	DropRequest.ScatterRadius = DropScatterRadius;
	DropRequest.WorldItemActorClass = WorldItemActorClass;
	DropRequest.bStartAutoCollect = true;
	DropRequest.AutoCollectMinDelay = AutoCollectMinDelay;
	DropRequest.AutoCollectMaxDelay = AutoCollectMaxDelay;
	DropRequest.AutoCollectRadius = AutoCollectRadius;

	APWWorldItemActor* WorldItem = UPWWorldItemDropLibrary::SpawnWorldItemDrop(this, DropRequest);
	if (!WorldItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Harvest cluster reward failed to spawn world item. Reward=%s x%d"),
			*GrantedRewardName.ToString(),
			GrantedRewardAmount);
		GrantRewardDirectDelayed(RewardReceiver, GrantedRewardName, GrantedRewardAmount);
		return;
	}

	UE_LOG(LogTemp, Verbose, TEXT("Harvest cluster reward spawned world item. Reward=%s x%d Receiver=%s ReceiverDistance=%.1f"),
		*GrantedRewardName.ToString(),
		GrantedRewardAmount,
		*GetNameSafe(RewardReceiver),
		FVector::Dist(RewardReceiver->GetActorLocation(), WorldItem->GetActorLocation()));
}
