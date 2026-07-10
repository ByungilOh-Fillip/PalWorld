#include "PWInteractionScannerComponent.h"

#include "PWHoldInteractable.h"
#include "PWInteractable.h"
#include "PWInteractableTargetComponent.h"
#include "PWLocalInteractable.h"
#include "PWMultiInteractable.h"
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

	if (ShouldUpdateLocalInteractionGuide())
	{
		ScanForInteractables();
	}
}

void UPWInteractionScannerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ShouldUpdateLocalInteractionGuide())
	{
		SetCurrentInteractableActor(nullptr);
	}

	Super::EndPlay(EndPlayReason);
}

void UPWInteractionScannerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ShouldUpdateLocalInteractionGuide())
	{
		return;
	}

	UpdateHoldGuideProgress(DeltaTime);

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

void UPWInteractionScannerComponent::GetCurrentInteractionGuideActions(TArray<FPWInteractionGuideAction>& OutActions) const
{
	GetInteractionGuideActions(CurrentInteractableActor.Get(), OutActions);
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

bool UPWInteractionScannerComponent::TryInteractByKey(FKey Key)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Key.IsValid())
	{
		return false;
	}

	if (Owner->HasAuthority())
	{
		return ExecuteInteractionByKey(FindBestInteractable(), Key.GetFName());
	}

	if (CurrentInteractableActor.IsValid())
	{
		ServerTryInteractByKey(Key.GetFName());
		return true;
	}

	return false;
}

bool UPWInteractionScannerComponent::TryInteractByActionId(FName ActionId)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || ActionId.IsNone())
	{
		return false;
	}

	if (Owner->HasAuthority())
	{
		return ExecuteInteractionAction(FindBestInteractable(), ActionId);
	}

	if (CurrentInteractableActor.IsValid())
	{
		ServerTryInteractByActionId(ActionId);
		return true;
	}

	return false;
}

bool UPWInteractionScannerComponent::TryBeginHoldInteraction()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}

	AActor* InteractableActor = Owner->HasAuthority() ? FindBestInteractable() : CurrentInteractableActor.Get();
	if (InteractableActor == nullptr)
	{
		return false;
	}

	const bool bCanBeginHold = InteractableActor->GetClass()->ImplementsInterface(UPWHoldInteractable::StaticClass())
		&& IPWHoldInteractable::Execute_CanBeginHoldInteraction(InteractableActor, Owner);
	if (Owner->HasAuthority())
	{
		if (bCanBeginHold)
		{
			StartHoldGuideProgress(InteractableActor);
			return ExecuteBeginHoldInteraction(InteractableActor);
		}

		return ExecuteLocalInteraction(InteractableActor) || ExecuteInteraction(InteractableActor);
	}

	if (bCanBeginHold)
	{
		StartHoldGuideProgress(InteractableActor);
		ServerTryBeginHoldInteraction();
		return true;
	}

	return ExecuteLocalInteraction(InteractableActor) || TryInteract();
}

void UPWInteractionScannerComponent::EndHoldInteraction()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	if (Owner->HasAuthority())
	{
		StopHoldGuideProgress();
		ExecuteEndHoldInteraction(CurrentHoldInteractableActor.Get());
		return;
	}

	StopHoldGuideProgress();
	ServerEndHoldInteraction();
}

void UPWInteractionScannerComponent::ServerTryInteract_Implementation()
{
	ExecuteInteraction(FindBestInteractable());
}

void UPWInteractionScannerComponent::ServerTryInteractByKey_Implementation(FName KeyName)
{
	ExecuteInteractionByKey(FindBestInteractable(), KeyName);
}

void UPWInteractionScannerComponent::ServerTryInteractByActionId_Implementation(FName ActionId)
{
	ExecuteInteractionAction(FindBestInteractable(), ActionId);
}

void UPWInteractionScannerComponent::ServerTryBeginHoldInteraction_Implementation()
{
	ExecuteBeginHoldInteraction(FindBestInteractable());
}

void UPWInteractionScannerComponent::ServerEndHoldInteraction_Implementation()
{
	ExecuteEndHoldInteraction(CurrentHoldInteractableActor.Get());
}

void UPWInteractionScannerComponent::ScanForInteractables()
{
	SetCurrentInteractableActor(FindBestInteractable());
	RefreshCurrentInteractionGuide();
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

	const FVector ScanOrigin = GetScanOrigin();
	const FVector ViewDirection = GetViewDirection();
	const FVector CandidateDirection = (CandidateComponent->GetInteractionLocation() - ScanOrigin).GetSafeNormal();
	const FVector BestDirection = (BestComponent->GetInteractionLocation() - ScanOrigin).GetSafeNormal();
	const float CandidateFacingScore = FVector::DotProduct(ViewDirection, CandidateDirection);
	const float BestFacingScore = FVector::DotProduct(ViewDirection, BestDirection);

	// 여러 대상이 겹칠 때는 플레이어가 바라보는 대상을 먼저 고르고,
	// 시선 차이가 애매할 때만 우선순위로 정렬한다.
	if (!FMath::IsNearlyEqual(CandidateFacingScore, BestFacingScore, 0.15f))
	{
		return CandidateFacingScore > BestFacingScore;
	}

	const int32 CandidatePriority = IPWInteractable::Execute_GetInteractionPriority(CandidateActor);
	const int32 BestPriority = IPWInteractable::Execute_GetInteractionPriority(BestActor);
	if (CandidatePriority != BestPriority)
	{
		return CandidatePriority > BestPriority;
	}

	const float CandidateDistanceSquared = FVector::DistSquared(ScanOrigin, CandidateComponent->GetInteractionLocation());
	const float BestDistanceSquared = FVector::DistSquared(ScanOrigin, BestComponent->GetInteractionLocation());
	if (!FMath::IsNearlyEqual(CandidateDistanceSquared, BestDistanceSquared, 1.0f))
	{
		return CandidateDistanceSquared < BestDistanceSquared;
	}

	return CurrentInteractableActor.Get() != BestActor && CurrentInteractableActor.Get() == CandidateActor;
}

bool UPWInteractionScannerComponent::ShouldUpdateLocalInteractionGuide() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	return OwnerPawn != nullptr && OwnerPawn->IsLocallyControlled();
}

void UPWInteractionScannerComponent::SetCurrentInteractableActor(AActor* NewInteractableActor)
{
	AActor* PreviousInteractableActor = CurrentInteractableActor.Get();
	if (PreviousInteractableActor == NewInteractableActor)
	{
		return;
	}

	HideInteractionGuide(PreviousInteractableActor);
	if (PreviousInteractableActor != nullptr && PreviousInteractableActor == CurrentHoldGuideActor.Get())
	{
		StopHoldGuideProgress();
	}
	CurrentInteractableActor = NewInteractableActor;
	ShowInteractionGuide(NewInteractableActor);
}

void UPWInteractionScannerComponent::RefreshCurrentInteractionGuide()
{
	AActor* InteractableActor = CurrentInteractableActor.Get();
	if (InteractableActor == nullptr)
	{
		return;
	}

	ShowInteractionGuide(InteractableActor);
}

void UPWInteractionScannerComponent::HideInteractionGuide(AActor* InteractableActor) const
{
	if (InteractableActor == nullptr)
	{
		return;
	}

	if (UPWInteractableTargetComponent* TargetComponent = InteractableActor->FindComponentByClass<UPWInteractableTargetComponent>())
	{
		TargetComponent->SetInteractionGuideVisible(false);
	}
}

void UPWInteractionScannerComponent::ShowInteractionGuide(AActor* InteractableActor) const
{
	if (InteractableActor == nullptr)
	{
		return;
	}

	UPWInteractableTargetComponent* TargetComponent = InteractableActor->FindComponentByClass<UPWInteractableTargetComponent>();
	if (TargetComponent == nullptr)
	{
		return;
	}

	TArray<FPWInteractionGuideAction> GuideActions;
	GetInteractionGuideActions(InteractableActor, GuideActions);
	TargetComponent->RefreshInteractionGuideWidget();
	TargetComponent->SetInteractionGuideVisible(GuideActions.Num() > 0);
}

void UPWInteractionScannerComponent::StartHoldGuideProgress(AActor* InteractableActor)
{
	if (!ShouldUpdateLocalInteractionGuide() || InteractableActor == nullptr)
	{
		return;
	}

	FPWInteractionGuideAction HoldGuideAction;
	if (!GetHoldGuideAction(InteractableActor, HoldGuideAction))
	{
		return;
	}

	StopHoldGuideProgress();

	CurrentHoldGuideActor = InteractableActor;
	CurrentHoldGuideActionId = HoldGuideAction.ActionId;
	CurrentHoldGuideDurationSeconds = FMath::Max(HoldGuideAction.ProgressDurationSeconds, KINDA_SMALL_NUMBER);
	const float InitialProgress = FMath::Clamp(HoldGuideAction.Progress, 0.0f, 1.0f);
	CurrentHoldGuideElapsedSeconds = InitialProgress * CurrentHoldGuideDurationSeconds;
	bHoldGuideProgressActive = true;

	if (UPWInteractableTargetComponent* TargetComponent = InteractableActor->FindComponentByClass<UPWInteractableTargetComponent>())
	{
		TargetComponent->SetInteractionGuideActionProgress(CurrentHoldGuideActionId, InitialProgress);
	}
}

void UPWInteractionScannerComponent::UpdateHoldGuideProgress(float DeltaTime)
{
	if (!bHoldGuideProgressActive)
	{
		return;
	}

	AActor* HoldGuideActor = CurrentHoldGuideActor.Get();
	if (HoldGuideActor == nullptr || CurrentHoldGuideActionId.IsNone())
	{
		StopHoldGuideProgress();
		return;
	}

	CurrentHoldGuideElapsedSeconds += DeltaTime;
	const float Progress = FMath::Clamp(CurrentHoldGuideElapsedSeconds / CurrentHoldGuideDurationSeconds, 0.0f, 1.0f);

	if (UPWInteractableTargetComponent* TargetComponent = HoldGuideActor->FindComponentByClass<UPWInteractableTargetComponent>())
	{
		TargetComponent->SetInteractionGuideActionProgress(CurrentHoldGuideActionId, Progress);
	}
}

void UPWInteractionScannerComponent::StopHoldGuideProgress()
{
	CurrentHoldGuideActor.Reset();
	CurrentHoldGuideActionId = NAME_None;
	CurrentHoldGuideElapsedSeconds = 0.0f;
	CurrentHoldGuideDurationSeconds = 1.0f;
	bHoldGuideProgressActive = false;
}

bool UPWInteractionScannerComponent::GetHoldGuideAction(AActor* InteractableActor, FPWInteractionGuideAction& OutAction) const
{
	TArray<FPWInteractionGuideAction> GuideActions;
	GetInteractionGuideActions(InteractableActor, GuideActions);

	for (const FPWInteractionGuideAction& GuideAction : GuideActions)
	{
		if (GuideAction.bEnabled && GuideAction.bShowProgress && GuideAction.Key == EKeys::F)
		{
			OutAction = GuideAction;
			return true;
		}
	}

	for (const FPWInteractionGuideAction& GuideAction : GuideActions)
	{
		if (GuideAction.bEnabled && GuideAction.bShowProgress)
		{
			OutAction = GuideAction;
			return true;
		}
	}

	return false;
}

void UPWInteractionScannerComponent::GetInteractionGuideActions(AActor* InteractableActor, TArray<FPWInteractionGuideAction>& OutActions) const
{
	OutActions.Reset();

	if (InteractableActor == nullptr)
	{
		return;
	}

	if (const UPWInteractableTargetComponent* TargetComponent = InteractableActor->FindComponentByClass<UPWInteractableTargetComponent>())
	{
		TargetComponent->GetInteractionGuideActions(OutActions);
	}
}

bool UPWInteractionScannerComponent::FindGuideActionByKey(AActor* InteractableActor, FName KeyName, FPWInteractionGuideAction& OutAction) const
{
	TArray<FPWInteractionGuideAction> GuideActions;
	GetInteractionGuideActions(InteractableActor, GuideActions);

	for (const FPWInteractionGuideAction& GuideAction : GuideActions)
	{
		if (GuideAction.bEnabled && GuideAction.Key.GetFName() == KeyName)
		{
			OutAction = GuideAction;
			return true;
		}
	}

	return false;
}

bool UPWInteractionScannerComponent::FindGuideActionByActionId(AActor* InteractableActor, FName ActionId, FPWInteractionGuideAction& OutAction) const
{
	TArray<FPWInteractionGuideAction> GuideActions;
	GetInteractionGuideActions(InteractableActor, GuideActions);

	for (const FPWInteractionGuideAction& GuideAction : GuideActions)
	{
		if (GuideAction.bEnabled && GuideAction.ActionId == ActionId)
		{
			OutAction = GuideAction;
			return true;
		}
	}

	return false;
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

bool UPWInteractionScannerComponent::ExecuteInteractionAction(AActor* InteractableActor, FName ActionId) const
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || InteractableActor == nullptr || ActionId.IsNone())
	{
		return false;
	}

	FPWInteractionGuideAction GuideAction;
	if (!FindGuideActionByActionId(InteractableActor, ActionId, GuideAction))
	{
		return false;
	}

	if (!InteractableActor->GetClass()->ImplementsInterface(UPWMultiInteractable::StaticClass()))
	{
		return ExecuteInteraction(InteractableActor);
	}

	const UPWInteractableTargetComponent* TargetComponent = InteractableActor->FindComponentByClass<UPWInteractableTargetComponent>();
	if (TargetComponent == nullptr || !TargetComponent->IsInteractionEnabled() || !IsInteractableInRange(InteractableActor, TargetComponent))
	{
		return false;
	}

	if (!IPWMultiInteractable::Execute_CanInteractAction(InteractableActor, Owner, ActionId))
	{
		return false;
	}

	return IPWMultiInteractable::Execute_InteractAction(InteractableActor, Owner, ActionId);
}

bool UPWInteractionScannerComponent::ExecuteInteractionByKey(AActor* InteractableActor, FName KeyName) const
{
	if (KeyName.IsNone())
	{
		return false;
	}

	FPWInteractionGuideAction GuideAction;
	if (!FindGuideActionByKey(InteractableActor, KeyName, GuideAction))
	{
		return false;
	}

	return ExecuteInteractionAction(InteractableActor, GuideAction.ActionId);
}

bool UPWInteractionScannerComponent::ExecuteLocalInteraction(AActor* InteractableActor) const
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || InteractableActor == nullptr || !InteractableActor->GetClass()->ImplementsInterface(UPWLocalInteractable::StaticClass()))
	{
		return false;
	}

	const UPWInteractableTargetComponent* TargetComponent = InteractableActor->FindComponentByClass<UPWInteractableTargetComponent>();
	if (TargetComponent == nullptr || !TargetComponent->IsInteractionEnabled() || !IsInteractableInRange(InteractableActor, TargetComponent))
	{
		return false;
	}

	if (!IPWLocalInteractable::Execute_CanLocalInteract(InteractableActor, Owner))
	{
		return false;
	}

	return IPWLocalInteractable::Execute_LocalInteract(InteractableActor, Owner);
}

bool UPWInteractionScannerComponent::ExecuteBeginHoldInteraction(AActor* InteractableActor)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || InteractableActor == nullptr || !InteractableActor->GetClass()->ImplementsInterface(UPWHoldInteractable::StaticClass()))
	{
		return false;
	}

	const UPWInteractableTargetComponent* TargetComponent = InteractableActor->FindComponentByClass<UPWInteractableTargetComponent>();
	if (TargetComponent == nullptr || !TargetComponent->IsInteractionEnabled() || !IsInteractableInRange(InteractableActor, TargetComponent))
	{
		return false;
	}

	if (!IPWHoldInteractable::Execute_CanBeginHoldInteraction(InteractableActor, Owner))
	{
		return false;
	}

	if (!IPWHoldInteractable::Execute_BeginHoldInteraction(InteractableActor, Owner))
	{
		return false;
	}

	CurrentHoldInteractableActor = InteractableActor;
	return true;
}

void UPWInteractionScannerComponent::ExecuteEndHoldInteraction(AActor* InteractableActor)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || InteractableActor == nullptr || !InteractableActor->GetClass()->ImplementsInterface(UPWHoldInteractable::StaticClass()))
	{
		CurrentHoldInteractableActor.Reset();
		return;
	}

	IPWHoldInteractable::Execute_EndHoldInteraction(InteractableActor, Owner);
	CurrentHoldInteractableActor.Reset();
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
