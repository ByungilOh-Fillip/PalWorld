#include "Resource/PW_HarvestableResourceActor.h"

#include "Components/StaticMeshComponent.h"
#include "Resource/PW_HarvestableResourceComponent.h"

APW_HarvestableResourceActor::APW_HarvestableResourceActor()
{
	bReplicates = true;
	SetReplicateMovement(false);

	ResourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ResourceMesh"));
	SetRootComponent(ResourceMesh);
	ResourceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ResourceMesh->SetCollisionResponseToAllChannels(ECR_Block);

	ResourceComponent = CreateDefaultSubobject<UPW_HarvestableResourceComponent>(TEXT("ResourceComponent"));
}

bool APW_HarvestableResourceActor::ApplyHarvestDamage_Implementation(float DamageAmount, AActor* InstigatorActor)
{
	return ResourceComponent != nullptr && ResourceComponent->ApplyHarvestDamage(DamageAmount, InstigatorActor);
}

void APW_HarvestableResourceActor::BeginPlay()
{
	Super::BeginPlay();

	if (ResourceComponent != nullptr)
	{
		ResourceComponent->OnDepletedStateChanged.AddDynamic(this, &APW_HarvestableResourceActor::HandleDepletedStateChanged);
		ApplyDepletedVisualState(ResourceComponent->IsDepleted());
	}
}

void APW_HarvestableResourceActor::HandleDepletedStateChanged(bool bNewIsDepleted)
{
	ApplyDepletedVisualState(bNewIsDepleted);
}

void APW_HarvestableResourceActor::ApplyDepletedVisualState(bool bNewIsDepleted)
{
	if (ResourceMesh == nullptr)
	{
		return;
	}

	ResourceMesh->SetHiddenInGame(bNewIsDepleted);
	ResourceMesh->SetVisibility(!bNewIsDepleted, true);
	ResourceMesh->SetCollisionEnabled(bNewIsDepleted ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
}
