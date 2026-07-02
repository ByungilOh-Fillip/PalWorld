#include "Base/PW_StorageBoxActor.h"

#include "Base/PW_BaseCampActor.h"
#include "Base/PW_BaseCampSubsystem.h"
#include "Base/PW_BaseInventoryAggregatorComponent.h"
#include "Base/PW_InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "PWInteractableTargetComponent.h"

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
	InteractableTargetComponent = CreateDefaultSubobject<UPWInteractableTargetComponent>(TEXT("InteractableTargetComponent"));
	InteractableTargetComponent->SetInteractionRadius(250.0f);
	InteractableTargetComponent->SetPromptText(NSLOCTEXT("PWInteraction", "StorageBoxPrompt", "Open Storage Box"));
	InteractableTargetComponent->SetPriority(100);
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
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(BaseCampRegistrationRetryTimerHandle);
		}

		UnregisterFromBaseCamp();
	}

	Super::EndPlay(EndPlayReason);
}

void APW_StorageBoxActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APW_StorageBoxActor, OwningBaseCamp);
}

bool APW_StorageBoxActor::CanInteract_Implementation(AActor* Interactor) const
{
	return Interactor != nullptr && InteractableTargetComponent != nullptr && InteractableTargetComponent->IsInteractionEnabled();
}

bool APW_StorageBoxActor::Interact_Implementation(AActor* Interactor)
{
	if (!CanInteract_Implementation(Interactor))
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[PWInteraction] Storage box interacted. Storage=%s Interactor=%s"),
		*GetName(),
		*GetNameSafe(Interactor));

	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Green,
			FString::Printf(TEXT("[Interaction] StorageBox: %s"), *GetName()));
	}

	return true;
}

FText APW_StorageBoxActor::GetInteractionPrompt_Implementation() const
{
	return InteractableTargetComponent ? InteractableTargetComponent->GetPromptText() : NSLOCTEXT("PWInteraction", "StorageBoxPromptFallback", "Open Storage Box");
}

int32 APW_StorageBoxActor::GetInteractionPriority_Implementation() const
{
	return InteractableTargetComponent ? InteractableTargetComponent->GetPriority() : 100;
}

void APW_StorageBoxActor::RegisterWithBaseCamp()
{
	UWorld* World = GetWorld();
	UPW_BaseCampSubsystem* BaseCampSubsystem = World != nullptr ? World->GetSubsystem<UPW_BaseCampSubsystem>() : nullptr;
	OwningBaseCamp = BaseCampSubsystem != nullptr ? BaseCampSubsystem->FindBaseCampAtLocation(GetActorLocation()) : nullptr;
	if (OwningBaseCamp != nullptr && OwningBaseCamp->GetInventoryAggregatorComponent() != nullptr)
	{
		OwningBaseCamp->GetInventoryAggregatorComponent()->RegisterStorageBox(this);
		if (World != nullptr)
		{
			World->GetTimerManager().ClearTimer(BaseCampRegistrationRetryTimerHandle);
		}
		return;
	}

	ScheduleBaseCampRegistrationRetry();
}

void APW_StorageBoxActor::ScheduleBaseCampRegistrationRetry()
{
	UWorld* World = GetWorld();
	if (World == nullptr || World->GetTimerManager().IsTimerActive(BaseCampRegistrationRetryTimerHandle))
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		BaseCampRegistrationRetryTimerHandle,
		this,
		&APW_StorageBoxActor::RegisterWithBaseCamp,
		0.5f,
		true);
}

void APW_StorageBoxActor::UnregisterFromBaseCamp()
{
	if (OwningBaseCamp != nullptr && OwningBaseCamp->GetInventoryAggregatorComponent() != nullptr)
	{
		OwningBaseCamp->GetInventoryAggregatorComponent()->UnregisterStorageBox(this);
	}

	OwningBaseCamp = nullptr;
}
