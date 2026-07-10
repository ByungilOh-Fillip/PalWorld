#include "Map/PW_TeleportPointActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "PWInteractableTargetComponent.h"
#include "UI/PW_WorldMapControllerComponent.h"

APW_TeleportPointActor::APW_TeleportPointActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(1.0f);
	SetMinNetUpdateFrequency(0.2f);
#if WITH_EDITOR
	bIsSpatiallyLoaded = false;
#endif

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(SceneRoot);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractableTargetComponent = CreateDefaultSubobject<UPWInteractableTargetComponent>(TEXT("InteractableTargetComponent"));
	InteractableTargetComponent->SetInteractionRadius(350.0f);
	InteractableTargetComponent->SetPromptText(NSLOCTEXT("PWInteraction", "TeleportPointPrompt", "Open Fast Travel"));
	InteractableTargetComponent->SetPriority(60);
}

void APW_TeleportPointActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && TeleportPointId.IsNone())
	{
		TeleportPointId = FName(*GetName());
		ForceNetUpdate();
	}
}

void APW_TeleportPointActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APW_TeleportPointActor, TeleportPointId);
	DOREPLIFETIME(APW_TeleportPointActor, bDiscovered);
	DOREPLIFETIME(APW_TeleportPointActor, bCanTeleport);
}

void APW_TeleportPointActor::SetDiscovered(bool bNewDiscovered)
{
	if (HasAuthority())
	{
		bDiscovered = bNewDiscovered;
		ForceNetUpdate();
	}
}

bool APW_TeleportPointActor::CanInteract_Implementation(AActor* Interactor) const
{
	return CanUseAsTeleportSource(Interactor);
}

bool APW_TeleportPointActor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || !CanInteract_Implementation(Interactor))
	{
		return false;
	}

	SetDiscovered(true);

	const APawn* InteractingPawn = Cast<APawn>(Interactor);
	APlayerController* PlayerController = InteractingPawn != nullptr ? Cast<APlayerController>(InteractingPawn->GetController()) : Cast<APlayerController>(Interactor);
	UPW_WorldMapControllerComponent* WorldMapController = PlayerController != nullptr ? PlayerController->FindComponentByClass<UPW_WorldMapControllerComponent>() : nullptr;
	if (WorldMapController == nullptr)
	{
		return false;
	}

	WorldMapController->SetActiveTeleportSource(this);
	WorldMapController->ClientShowTeleportMap();
	return true;
}

FText APW_TeleportPointActor::GetInteractionPrompt_Implementation() const
{
	return InteractableTargetComponent != nullptr
		? InteractableTargetComponent->GetPromptText()
		: NSLOCTEXT("PWInteraction", "TeleportPointPromptFallback", "Open Fast Travel");
}

int32 APW_TeleportPointActor::GetInteractionPriority_Implementation() const
{
	return InteractableTargetComponent != nullptr ? InteractableTargetComponent->GetPriority() : 60;
}

bool APW_TeleportPointActor::CanUseAsTeleportSource(AActor* Interactor) const
{
	if (Interactor == nullptr || InteractableTargetComponent == nullptr || !InteractableTargetComponent->IsInteractionEnabled())
	{
		return false;
	}

	const float DistanceSquared = FVector::DistSquared(Interactor->GetActorLocation(), InteractableTargetComponent->GetInteractionLocation());
	return DistanceSquared <= FMath::Square(InteractableTargetComponent->GetInteractionRadius());
}

FVector APW_TeleportPointActor::GetTeleportArrivalLocation() const
{
	return GetActorLocation()
		+ GetActorForwardVector() * ArrivalForwardOffset
		+ FVector::UpVector * ArrivalUpOffset;
}
