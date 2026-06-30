#include "Base/PW_StorageBoxActor.h"

#include "Base/PW_BaseCampActor.h"
#include "Base/PW_BaseCampSubsystem.h"
#include "Base/PW_BaseInventoryAggregatorComponent.h"
#include "Base/PW_InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

APW_StorageBoxActor::APW_StorageBoxActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetNetCullDistanceSquared(FMath::Square(8000.0f));
	NetUpdateFrequency = 2.0f;

	StorageMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StorageMesh"));
	SetRootComponent(StorageMesh);
	StorageMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	StorageMesh->SetCanEverAffectNavigation(true);

	InventoryComponent = CreateDefaultSubobject<UPW_InventoryComponent>(TEXT("InventoryComponent"));
}

void APW_StorageBoxActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		RegisterWithBaseCamp();
	}
}

void APW_StorageBoxActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		UnregisterFromBaseCamp();
	}

	Super::EndPlay(EndPlayReason);
}

void APW_StorageBoxActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APW_StorageBoxActor, OwningBaseCamp);
}

void APW_StorageBoxActor::RegisterWithBaseCamp()
{
	UWorld* World = GetWorld();
	UPW_BaseCampSubsystem* BaseCampSubsystem = World != nullptr ? World->GetSubsystem<UPW_BaseCampSubsystem>() : nullptr;
	OwningBaseCamp = BaseCampSubsystem != nullptr ? BaseCampSubsystem->FindBaseCampAtLocation(GetActorLocation()) : nullptr;
	if (OwningBaseCamp != nullptr && OwningBaseCamp->GetInventoryAggregatorComponent() != nullptr)
	{
		OwningBaseCamp->GetInventoryAggregatorComponent()->RegisterStorageBox(this);
	}
}

void APW_StorageBoxActor::UnregisterFromBaseCamp()
{
	if (OwningBaseCamp != nullptr && OwningBaseCamp->GetInventoryAggregatorComponent() != nullptr)
	{
		OwningBaseCamp->GetInventoryAggregatorComponent()->UnregisterStorageBox(this);
	}

	OwningBaseCamp = nullptr;
}
