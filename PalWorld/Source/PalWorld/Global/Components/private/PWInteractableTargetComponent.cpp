#include "PWInteractableTargetComponent.h"

#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "PWInteractionGuideWidget.h"

UPWInteractableTargetComponent::UPWInteractableTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPWInteractableTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureInteractionGuideWidgetComponent();
	RefreshInteractionGuideWidget();
}

void UPWInteractableTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InteractionGuideWidgetComponent)
	{
		InteractionGuideWidgetComponent->DestroyComponent();
		InteractionGuideWidgetComponent = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UPWInteractableTargetComponent::SetInteractionEnabled(bool bNewInteractionEnabled)
{
	bInteractionEnabled = bNewInteractionEnabled;
	RefreshInteractionGuideWidget();
}

void UPWInteractableTargetComponent::SetInteractionRadius(float NewInteractionRadius)
{
	InteractionRadius = FMath::Max(0.0f, NewInteractionRadius);
}

void UPWInteractableTargetComponent::SetPromptText(const FText& NewPromptText)
{
	PromptText = NewPromptText;
	RefreshInteractionGuideWidget();
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

void UPWInteractableTargetComponent::SetInteractionGuideVisible(bool bVisible)
{
	EnsureInteractionGuideWidgetComponent();

	if (InteractionGuideWidgetComponent)
	{
		InteractionGuideWidgetComponent->SetVisibility(bVisible);
	}
}

void UPWInteractableTargetComponent::SetInteractionGuideActions(const TArray<FPWInteractionGuideAction>& NewGuideActions)
{
	InteractionGuideActions = NewGuideActions;
	RefreshInteractionGuideWidget();
}

void UPWInteractableTargetComponent::GetInteractionGuideActions(TArray<FPWInteractionGuideAction>& OutActions) const
{
	OutActions.Reset();

	if (InteractionGuideActions.Num() > 0)
	{
		OutActions = InteractionGuideActions;
	}
	else if (!PromptText.IsEmpty())
	{
		FPWInteractionGuideAction Action;
		Action.ActionId = DefaultActionId;
		Action.Key = DefaultActionKey;
		Action.Label = PromptText;
		Action.bEnabled = bInteractionEnabled;
		Action.SortOrder = 0;
		OutActions.Add(Action);
	}

	for (FPWInteractionGuideAction& Action : OutActions)
	{
		if (const float* Progress = InteractionGuideActionProgressById.Find(Action.ActionId))
		{
			Action.Progress = FMath::Clamp(*Progress, 0.0f, 1.0f);
			Action.bShowProgress = true;
		}
	}

	OutActions.RemoveAll([](const FPWInteractionGuideAction& Action)
	{
		return !Action.IsValidGuide();
	});

	OutActions.Sort([](const FPWInteractionGuideAction& Left, const FPWInteractionGuideAction& Right)
	{
		return Left.SortOrder < Right.SortOrder;
	});
}

void UPWInteractableTargetComponent::SetInteractionGuideActionProgress(FName ActionId, float NewProgress)
{
	if (ActionId.IsNone())
	{
		return;
	}

	InteractionGuideActionProgressById.FindOrAdd(ActionId) = FMath::Clamp(NewProgress, 0.0f, 1.0f);
	RefreshInteractionGuideWidget();
}

void UPWInteractableTargetComponent::ClearInteractionGuideActionProgress(FName ActionId)
{
	if (ActionId.IsNone())
	{
		return;
	}

	InteractionGuideActionProgressById.Remove(ActionId);
	RefreshInteractionGuideWidget();
}

void UPWInteractableTargetComponent::RefreshInteractionGuideWidget()
{
	if (!HasBegunPlay())
	{
		return;
	}

	TArray<FPWInteractionGuideAction> GuideActions;
	GetInteractionGuideActions(GuideActions);
	ApplyInteractionGuideWidgetActions(GuideActions);
}

void UPWInteractableTargetComponent::ApplyInteractionGuideWidgetActions(const TArray<FPWInteractionGuideAction>& GuideActions)
{
	EnsureInteractionGuideWidgetComponent();

	if (InteractionGuideWidgetComponent == nullptr)
	{
		return;
	}

	if (UPWInteractionGuideWidget* GuideWidget = Cast<UPWInteractionGuideWidget>(InteractionGuideWidgetComponent->GetUserWidgetObject()))
	{
		GuideWidget->SetGuideActions(GuideActions);
	}
}

void UPWInteractableTargetComponent::EnsureInteractionGuideWidgetComponent()
{
	if (!HasBegunPlay() || InteractionGuideWidgetComponent != nullptr || GetOwner() == nullptr)
	{
		return;
	}

	USceneComponent* AttachParent = GetOwner()->GetRootComponent();
	if (AttachParent == nullptr)
	{
		return;
	}

	InteractionGuideWidgetComponent = NewObject<UWidgetComponent>(GetOwner(), TEXT("InteractionGuideWidgetComponent"));
	if (InteractionGuideWidgetComponent == nullptr)
	{
		return;
	}

	InteractionGuideWidgetComponent->SetupAttachment(AttachParent);
	InteractionGuideWidgetComponent->SetRelativeLocation(InteractionPointOffset + GuideWidgetOffset);
	InteractionGuideWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionGuideWidgetComponent->SetDrawAtDesiredSize(true);
	InteractionGuideWidgetComponent->SetDrawSize(GuideWidgetDrawSize);
	InteractionGuideWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionGuideWidgetComponent->SetVisibility(false);

	UClass* WidgetClass = InteractionGuideWidgetClass ? InteractionGuideWidgetClass.Get() : UPWInteractionGuideWidget::StaticClass();
	InteractionGuideWidgetComponent->SetWidgetClass(WidgetClass);

	InteractionGuideWidgetComponent->RegisterComponent();
	InteractionGuideWidgetComponent->InitWidget();
}
