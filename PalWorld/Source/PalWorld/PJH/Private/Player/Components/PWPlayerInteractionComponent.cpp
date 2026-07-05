// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerInteractionComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "PWInteractable.h"
#include "PWInteractableTargetComponent.h"
#include "World/PWWorldItemActor.h"

UPWPlayerInteractionComponent::UPWPlayerInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UPWPlayerInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	TimeUntilNextSearch = 0.f;
}

void UPWPlayerInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetActorHighlighted(CurrentInteractable.Get(), false);
	CurrentInteractable.Reset();

	Super::EndPlay(EndPlayReason);
}

void UPWPlayerInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsLocallyControlledOwner())
	{
		return;
	}

	TimeUntilNextSearch -= DeltaTime;
	if (TimeUntilNextSearch > 0.f)
	{
		return;
	}

	TimeUntilNextSearch = SearchInterval;
	UpdateCurrentInteractable();
}

bool UPWPlayerInteractionComponent::TryInteract()
{
	AActor* TargetActor = CurrentInteractable.Get();
	if (!IsInteractableAllowed(TargetActor))
	{
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	if (!OwnerActor->HasAuthority())
	{
		ServerInteractWithActor(TargetActor);
		return true;
	}

	return IPWInteractable::Execute_Interact(TargetActor, OwnerActor);
}

void UPWPlayerInteractionComponent::ServerInteractWithActor_Implementation(AActor* TargetActor)
{
	if (!IsInteractableAllowed(TargetActor))
	{
		return;
	}

	IPWInteractable::Execute_Interact(TargetActor, GetOwner());
}

void UPWPlayerInteractionComponent::UpdateCurrentInteractable()
{
	AActor* PreviousInteractable = CurrentInteractable.Get();
	AActor* NewInteractable = FindBestInteractable();
	if (PreviousInteractable == NewInteractable)
	{
		return;
	}

	SetActorHighlighted(PreviousInteractable, false);
	CurrentInteractable = NewInteractable;
	SetActorHighlighted(NewInteractable, true);
}

AActor* UPWPlayerInteractionComponent::FindBestInteractable() const
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor)
	{
		return nullptr;
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWPlayerInteractionSearch), false, OwnerActor);
	World->OverlapMultiByObjectType(
		OverlapResults,
		OwnerActor->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(SearchRadius),
		QueryParams);

	AActor* BestTarget = nullptr;
	float BestScore = -FLT_MAX;

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* Candidate = OverlapResult.GetActor();
		if (!IsInteractableAllowed(Candidate))
		{
			continue;
		}

		const float Score = GetInteractableScore(Candidate);
		if (Score > BestScore)
		{
			BestTarget = Candidate;
			BestScore = Score;
		}
	}

	return BestTarget;
}

bool UPWPlayerInteractionComponent::IsLocallyControlledOwner() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	return OwnerPawn && OwnerPawn->IsLocallyControlled();
}

bool UPWPlayerInteractionComponent::IsInteractableAllowed(AActor* TargetActor) const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !TargetActor || TargetActor == OwnerActor)
	{
		return false;
	}

	if (!TargetActor->GetClass()->ImplementsInterface(UPWInteractable::StaticClass()))
	{
		return false;
	}

	if (!IPWInteractable::Execute_CanInteract(TargetActor, OwnerActor))
	{
		return false;
	}

	UPWInteractableTargetComponent* TargetComponent = TargetActor->FindComponentByClass<UPWInteractableTargetComponent>();
	const float AllowedRadius = TargetComponent ? TargetComponent->GetInteractionRadius() : SearchRadius;
	return FVector::DistSquared(OwnerActor->GetActorLocation(), TargetActor->GetActorLocation()) <= FMath::Square(FMath::Max(SearchRadius, AllowedRadius));
}

float UPWPlayerInteractionComponent::GetInteractableScore(AActor* TargetActor) const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !TargetActor)
	{
		return -FLT_MAX;
	}

	const int32 Priority = IPWInteractable::Execute_GetInteractionPriority(TargetActor);
	const float DistanceSquared = FVector::DistSquared(OwnerActor->GetActorLocation(), TargetActor->GetActorLocation());
	return static_cast<float>(Priority) * 100000.f - DistanceSquared;
}

void UPWPlayerInteractionComponent::SetActorHighlighted(AActor* TargetActor, bool bHighlighted) const
{
	if (!bUseCustomDepthHighlight || !TargetActor)
	{
		return;
	}

	if (APWWorldItemActor* WorldItemActor = Cast<APWWorldItemActor>(TargetActor))
	{
		WorldItemActor->SetHighlighted(bHighlighted);
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		PrimitiveComponent->SetRenderCustomDepth(bHighlighted);
		PrimitiveComponent->SetCustomDepthStencilValue(HighlightStencilValue);
	}
}
