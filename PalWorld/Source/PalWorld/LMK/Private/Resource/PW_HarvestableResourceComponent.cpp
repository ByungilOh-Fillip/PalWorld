#include "Resource/PW_HarvestableResourceComponent.h"

#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Interfaces/PW_ItemReceiver.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPlayerInventoryLinkComponent.h"
#include "Player/Data/PWItemDataAsset.h"
#include "TimerManager.h"
#include "World/PWWorldItemDropLibrary.h"
#include "World/PWWorldItemActor.h"

UPW_HarvestableResourceComponent::UPW_HarvestableResourceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	WorldItemActorClass = APWWorldItemActor::StaticClass();
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

AActor* UPW_HarvestableResourceComponent::ResolveRewardReceiver(AActor* InstigatorActor) const
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

UPWItemDataAsset* UPW_HarvestableResourceComponent::ResolveRewardItemData(AActor* RewardReceiver, FName GrantedRewardName) const
{
	const UPWPlayerInventoryLinkComponent* InventoryComponent = RewardReceiver ? RewardReceiver->FindComponentByClass<UPWPlayerInventoryLinkComponent>() : nullptr;
	return InventoryComponent ? InventoryComponent->GetItemDefinition(GrantedRewardName) : nullptr;
}

void UPW_HarvestableResourceComponent::GrantRewardDirectDelayed(AActor* RewardReceiver, FName GrantedRewardName, int32 GrantedRewardAmount)
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

void UPW_HarvestableResourceComponent::GrantReward(AActor* InstigatorActor, int32 RewardMultiplier)
{
	const int32 GrantedRewardAmount = RewardMultiplier * RewardAmount;
	if (!InstigatorActor || GrantedRewardAmount <= 0 || RewardName.IsNone())
	{
		return;
	}

	AActor* RewardReceiver = ResolveRewardReceiver(InstigatorActor);
	if (!RewardReceiver)
	{
		UE_LOG(LogTemp, Warning, TEXT("Harvest reward has no receiver. Instigator=%s Reward=%s x%d"),
			*InstigatorActor->GetName(),
			*RewardName.ToString(),
			GrantedRewardAmount);
		return;
	}

	UWorld* World = GetWorld();
	if (!bSpawnWorldDrop || !World)
	{
		UE_LOG(LogTemp, Warning, TEXT("Harvest reward uses direct collect fallback. SpawnWorldDrop=%s World=%s"),
			bSpawnWorldDrop ? TEXT("true") : TEXT("false"),
			World ? TEXT("valid") : TEXT("none"));
		GrantRewardDirectDelayed(RewardReceiver, RewardName, GrantedRewardAmount);
		return;
	}

	const FVector ResourceDropLocation = GetOwner()->GetActorLocation() + DropLocationOffset;
	UPWItemDataAsset* RewardItemData = ResolveRewardItemData(RewardReceiver, RewardName);
	if (!UPWWorldItemDropLibrary::CanSpawnWorldItem(RewardItemData))
	{
		UE_LOG(LogTemp, Warning, TEXT("Harvest reward has no valid world item data. Reward=%s ItemData=%s WorldMesh=%s. Use direct collect fallback."),
			*RewardName.ToString(),
			*GetNameSafe(RewardItemData),
			RewardItemData && RewardItemData->GetWorldMesh() ? *RewardItemData->GetWorldMesh()->GetName() : TEXT("None"));
		GrantRewardDirectDelayed(RewardReceiver, RewardName, GrantedRewardAmount);
		return;
	}

	FPWWorldItemDropRequest DropRequest;
	DropRequest.ItemData = RewardItemData;
	DropRequest.ItemId = RewardName;
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
		UE_LOG(LogTemp, Warning, TEXT("Harvest reward failed to spawn world item. Reward=%s x%d"),
			*RewardName.ToString(),
			GrantedRewardAmount);
		GrantRewardDirectDelayed(RewardReceiver, RewardName, GrantedRewardAmount);
		return;
	}

	UE_LOG(LogTemp, Verbose, TEXT("Harvest reward spawned world item. Reward=%s x%d Receiver=%s ReceiverDistance=%.1f"),
		*RewardName.ToString(),
		GrantedRewardAmount,
		*GetNameSafe(RewardReceiver),
		FVector::Dist(RewardReceiver->GetActorLocation(), WorldItem->GetActorLocation()));
}
