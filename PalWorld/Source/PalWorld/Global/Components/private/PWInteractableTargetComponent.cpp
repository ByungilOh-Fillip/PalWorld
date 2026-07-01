#include "PWInteractableTargetComponent.h"

UPWInteractableTargetComponent::UPWInteractableTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPWInteractableTargetComponent::SetInteractionEnabled(bool bNewInteractionEnabled)
{
	bInteractionEnabled = bNewInteractionEnabled;
}

void UPWInteractableTargetComponent::SetInteractionRadius(float NewInteractionRadius)
{
	InteractionRadius = FMath::Max(0.0f, NewInteractionRadius);
}

void UPWInteractableTargetComponent::SetPromptText(const FText& NewPromptText)
{
	PromptText = NewPromptText;
}

void UPWInteractableTargetComponent::SetPriority(int32 NewPriority)
{
	Priority = NewPriority;
}

FVector UPWInteractableTargetComponent::GetInteractionLocation() const
{
	const AActor* Owner = GetOwner();
	return Owner ? Owner->GetActorTransform().TransformPosition(InteractionPointOffset) : InteractionPointOffset;
}
