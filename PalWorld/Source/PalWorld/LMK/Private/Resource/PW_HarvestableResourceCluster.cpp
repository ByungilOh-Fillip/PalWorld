#include "Resource/PW_HarvestableResourceCluster.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Resource/PW_HarvestableResourceClusterComponent.h"

APW_HarvestableResourceCluster::APW_HarvestableResourceCluster()
{
	bReplicates = true;
	SetReplicateMovement(false);

	HarvestInstances = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HarvestInstances"));
	SetRootComponent(HarvestInstances);
	HarvestInstances->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HarvestInstances->SetCollisionResponseToAllChannels(ECR_Block);

	ClusterComponent = CreateDefaultSubobject<UPW_HarvestableResourceClusterComponent>(TEXT("ClusterComponent"));
}

void APW_HarvestableResourceCluster::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RebuildInstances();
}

bool APW_HarvestableResourceCluster::ApplyHarvestDamageToInstance_Implementation(
	int32 InstanceIndex,
	float DamageAmount,
	AActor* InstigatorActor)
{
	return ClusterComponent != nullptr
		&& ClusterComponent->ApplyHarvestDamageToInstance(InstanceIndex, DamageAmount, InstigatorActor);
}

void APW_HarvestableResourceCluster::BeginPlay()
{
	Super::BeginPlay();

	RebuildInstances();

	if (ClusterComponent != nullptr)
	{
		ClusterComponent->InitializeInstances(InstanceTransforms.Num());
		ClusterComponent->OnInstanceDepleted.AddDynamic(this, &APW_HarvestableResourceCluster::HandleInstanceDepleted);
		ClusterComponent->OnInstanceRespawned.AddDynamic(this, &APW_HarvestableResourceCluster::HandleInstanceRespawned);
		ClusterComponent->OnReplicatedStateChanged.AddDynamic(this, &APW_HarvestableResourceCluster::HandleReplicatedStateChanged);
		ApplyReplicatedInstanceStates();
	}
}

void APW_HarvestableResourceCluster::HandleInstanceDepleted(int32 InstanceIndex, AActor* InstigatorActor)
{
	SetInstanceHidden(InstanceIndex, true);
	SpawnDepletionActor(InstanceIndex, InstigatorActor);
}

void APW_HarvestableResourceCluster::HandleInstanceRespawned(int32 InstanceIndex)
{
	SetInstanceHidden(InstanceIndex, false);
}

void APW_HarvestableResourceCluster::HandleReplicatedStateChanged()
{
	ApplyReplicatedInstanceStates();
}

void APW_HarvestableResourceCluster::RebuildInstances()
{
	if (HarvestInstances == nullptr)
	{
		return;
	}

	HarvestInstances->ClearInstances();
	for (const FTransform& InstanceTransform : InstanceTransforms)
	{
		HarvestInstances->AddInstance(InstanceTransform);
	}
}

void APW_HarvestableResourceCluster::ApplyReplicatedInstanceStates()
{
	if (ClusterComponent == nullptr)
	{
		return;
	}

	for (int32 InstanceIndex = 0; InstanceIndex < InstanceTransforms.Num(); ++InstanceIndex)
	{
		SetInstanceHidden(InstanceIndex, ClusterComponent->IsInstanceDepleted(InstanceIndex));
	}
}

void APW_HarvestableResourceCluster::SetInstanceHidden(int32 InstanceIndex, bool bShouldHide)
{
	if (HarvestInstances == nullptr || !InstanceTransforms.IsValidIndex(InstanceIndex))
	{
		return;
	}

	FTransform TargetTransform = InstanceTransforms[InstanceIndex];
	if (bShouldHide)
	{
		TargetTransform.SetScale3D(FVector::ZeroVector);
	}

	HarvestInstances->UpdateInstanceTransform(InstanceIndex, TargetTransform, false, true, true);
}

void APW_HarvestableResourceCluster::SpawnDepletionActor(int32 InstanceIndex, AActor* InstigatorActor)
{
	if (!HasAuthority() || DepletionActorClass == nullptr)
	{
		return;
	}

	FTransform SpawnTransform;
	if (!GetInstanceWorldTransform(InstanceIndex, SpawnTransform))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = Cast<APawn>(InstigatorActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AActor* DepletionActor = GetWorld()->SpawnActor<AActor>(DepletionActorClass, SpawnTransform, SpawnParams))
	{
		DepletionActor->SetReplicates(true);
		if (DepletionActorLifeTime > 0.0f)
		{
			DepletionActor->SetLifeSpan(DepletionActorLifeTime);
		}
	}
}

bool APW_HarvestableResourceCluster::GetInstanceWorldTransform(int32 InstanceIndex, FTransform& OutTransform) const
{
	if (!InstanceTransforms.IsValidIndex(InstanceIndex))
	{
		return false;
	}

	OutTransform = InstanceTransforms[InstanceIndex] * GetActorTransform();
	return true;
}
