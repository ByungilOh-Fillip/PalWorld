#include "PWInteractionScannerComponent.h"

#include "PWInteractable.h"
#include "PWInteractableTargetComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UPWInteractionScannerComponent::UPWInteractionScannerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UPWInteractionScannerComponent::BeginPlay()
{
	Super::BeginPlay();

	ScanForInteractables();
}

void UPWInteractionScannerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeUntilNextScan -= DeltaTime;
	if (TimeUntilNextScan > 0.0f)
	{
		return;
	}

	TimeUntilNextScan = ScanIntervalSeconds;
	ScanForInteractables();
}

FText UPWInteractionScannerComponent::GetCurrentPrompt() const
{
	AActor* InteractableActor = CurrentInteractableActor.Get();
	if (InteractableActor == nullptr || !InteractableActor->GetClass()->ImplementsInterface(UPWInteractable::StaticClass()))
	{
		return FText::GetEmpty();
	}

	return IPWInteractable::Execute_GetInteractionPrompt(InteractableActor);
}

bool UPWInteractionScannerComponent::TryInteract()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}

	if (Owner->HasAuthority())
	{
		return ExecuteInteraction(FindBestInteractable());
	}

	if (CurrentInteractableActor.IsValid())
	{
		ServerTryInteract();
		return true;
	}

	return false;
}

void UPWInteractionScannerComponent::ServerTryInteract_Implementation()
{
	ExecuteInteraction(FindBestInteractable());
}

void UPWInteractionScannerComponent::ScanForInteractables()
{
	CurrentInteractableActor = FindBestInteractable();
}

AActor* UPWInteractionScannerComponent::FindBestInteractable() const
{
	const UWorld* World = GetWorld();
	const AActor* Owner = GetOwner();
	if (World == nullptr || Owner == nullptr)
	{
		return nullptr;
	}

	TArray<AActor*> CandidateActors;
	UGameplayStatics::GetAllActorsWithInterface(World, UPWInteractable::StaticClass(), CandidateActors);

	AActor* BestActor = nullptr;
	UPWInteractableTargetComponent* BestComponent = nullptr;

	for (AActor* CandidateActor : CandidateActors)
	{
		if (CandidateActor == nullptr || CandidateActor == Owner)
		{
			continue;
		}

		UPWInteractableTargetComponent* TargetComponent = CandidateActor->FindComponentByClass<UPWInteractableTargetComponent>();
		if (TargetComponent == nullptr || !TargetComponent->IsInteractionEnabled() || !IsInteractableInRange(CandidateActor, TargetComponent))
		{
			continue;
		}

		if (!IPWInteractable::Execute_CanInteract(CandidateActor, const_cast<AActor*>(Owner)))
		{
			continue;
		}

		if (IsBetterInteractable(CandidateActor, TargetComponent, BestActor, BestComponent))
		{
			BestActor = CandidateActor;
			BestComponent = TargetComponent;
		}
	}

	return BestActor;
}

bool UPWInteractionScannerComponent::IsInteractableInRange(AActor* CandidateActor, const UPWInteractableTargetComponent* TargetComponent) const
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || CandidateActor == nullptr || TargetComponent == nullptr)
	{
		return false;
	}

	const float DistanceSquared = FVector::DistSquared(GetScanOrigin(), TargetComponent->GetInteractionLocation());
	return DistanceSquared <= FMath::Square(TargetComponent->GetInteractionRadius())
		&& DistanceSquared <= FMath::Square(ScanRadius);
}

bool UPWInteractionScannerComponent::IsBetterInteractable(
	AActor* CandidateActor,
	const UPWInteractableTargetComponent* CandidateComponent,
	AActor* BestActor,
	const UPWInteractableTargetComponent* BestComponent) const
{
	if (CandidateActor == nullptr || CandidateComponent == nullptr)
	{
		return false;
	}

	if (BestActor == nullptr || BestComponent == nullptr)
	{
		return true;
	}

	const int32 CandidatePriority = IPWInteractable::Execute_GetInteractionPriority(CandidateActor);
	const int32 BestPriority = IPWInteractable::Execute_GetInteractionPriority(BestActor);
	if (CandidatePriority != BestPriority)
	{
		return CandidatePriority > BestPriority;
	}

	const FVector ScanOrigin = GetScanOrigin();
	const FVector ViewDirection = GetViewDirection();
	const FVector CandidateDirection = (CandidateComponent->GetInteractionLocation() - ScanOrigin).GetSafeNormal();
	const FVector BestDirection = (BestComponent->GetInteractionLocation() - ScanOrigin).GetSafeNormal();
	const float CandidateFacingScore = FVector::DotProduct(ViewDirection, CandidateDirection);
	const float BestFacingScore = FVector::DotProduct(ViewDirection, BestDirection);
	if (!FMath::IsNearlyEqual(CandidateFacingScore, BestFacingScore, 0.01f))
	{
		return CandidateFacingScore > BestFacingScore;
	}

	const float CandidateDistanceSquared = FVector::DistSquared(ScanOrigin, CandidateComponent->GetInteractionLocation());
	const float BestDistanceSquared = FVector::DistSquared(ScanOrigin, BestComponent->GetInteractionLocation());
	if (!FMath::IsNearlyEqual(CandidateDistanceSquared, BestDistanceSquared, 1.0f))
	{
		return CandidateDistanceSquared < BestDistanceSquared;
	}

	return CurrentInteractableActor.Get() != BestActor && CurrentInteractableActor.Get() == CandidateActor;
}

bool UPWInteractionScannerComponent::ExecuteInteraction(AActor* InteractableActor) const
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || InteractableActor == nullptr || !InteractableActor->GetClass()->ImplementsInterface(UPWInteractable::StaticClass()))
	{
		return false;
	}

	const UPWInteractableTargetComponent* TargetComponent = InteractableActor->FindComponentByClass<UPWInteractableTargetComponent>();
	if (TargetComponent == nullptr || !TargetComponent->IsInteractionEnabled() || !IsInteractableInRange(InteractableActor, TargetComponent))
	{
		return false;
	}

	if (!IPWInteractable::Execute_CanInteract(InteractableActor, Owner))
	{
		return false;
	}

	return IPWInteractable::Execute_Interact(InteractableActor, Owner);
}

FVector UPWInteractionScannerComponent::GetScanOrigin() const
{
	const AActor* Owner = GetOwner();
	return Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
}

FVector UPWInteractionScannerComponent::GetViewDirection() const
{
	const AActor* Owner = GetOwner();
	const APawn* OwnerPawn = Cast<APawn>(Owner);
	const AController* Controller = OwnerPawn ? OwnerPawn->GetController() : nullptr;
	return Controller ? Controller->GetControlRotation().Vector() : (Owner ? Owner->GetActorForwardVector() : FVector::ForwardVector);
}
