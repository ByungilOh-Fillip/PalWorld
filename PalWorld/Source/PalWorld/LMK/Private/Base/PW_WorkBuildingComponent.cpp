#include "Base/PW_WorkBuildingComponent.h"

#include "Net/UnrealNetwork.h"
#include "PWInteractableTargetComponent.h"

UPW_WorkBuildingComponent::UPW_WorkBuildingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UPW_WorkBuildingComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner != nullptr && Owner->HasAuthority() && bStartWithReservedWork)
	{
		ReserveWork(TEXT("DebugWork"));
	}

	RefreshInteractionGuideProgress();
}

void UPW_WorkBuildingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AActor* Owner = GetOwner();
	if (Owner != nullptr && Owner->HasAuthority())
	{
		ClearActiveWorkers();
	}

	Super::EndPlay(EndPlayReason);
}

void UPW_WorkBuildingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (Owner != nullptr && Owner->HasAuthority())
	{
		ApplyActiveWorkerProgress(DeltaTime);
	}
}

void UPW_WorkBuildingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPW_WorkBuildingComponent, WorkId);
	DOREPLIFETIME(UPW_WorkBuildingComponent, WorkState);
	DOREPLIFETIME(UPW_WorkBuildingComponent, WorkProgress);
	DOREPLIFETIME(UPW_WorkBuildingComponent, bHasReservedWork);
	DOREPLIFETIME(UPW_WorkBuildingComponent, ActiveWorkerCount);
}

void UPW_WorkBuildingComponent::SetHasReservedWork(bool bNewHasReservedWork)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	if (bNewHasReservedWork)
	{
		ReserveWork(WorkId.IsNone() ? TEXT("ReservedWork") : WorkId);
		return;
	}

	CancelReservedWork();
}

bool UPW_WorkBuildingComponent::ReserveWork(FName NewWorkId)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || NewWorkId.IsNone() || WorkState == EPW_WorkBuildingState::InProgress)
	{
		return false;
	}

	WorkId = NewWorkId;
	WorkProgress = 0.0f;
	WorkState = EPW_WorkBuildingState::Reserved;
	bHasReservedWork = true;
	ClearActiveWorkers();
	RefreshInteractionGuideProgress();
	Owner->ForceNetUpdate();
	return true;
}

bool UPW_WorkBuildingComponent::CancelReservedWork()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || WorkState == EPW_WorkBuildingState::InProgress)
	{
		return false;
	}

	WorkId = NAME_None;
	WorkProgress = 0.0f;
	WorkState = EPW_WorkBuildingState::Idle;
	bHasReservedWork = false;
	ClearActiveWorkers();
	RefreshInteractionGuideProgress();
	Owner->ForceNetUpdate();
	return true;
}

bool UPW_WorkBuildingComponent::HasReservedOrActiveWork() const
{
	return WorkState == EPW_WorkBuildingState::Reserved || WorkState == EPW_WorkBuildingState::InProgress || ActiveWorkerCount > 0;
}

bool UPW_WorkBuildingComponent::IsWorkAvailable() const
{
	return WorkState == EPW_WorkBuildingState::Reserved || WorkState == EPW_WorkBuildingState::InProgress;
}

bool UPW_WorkBuildingComponent::CanBeginWork(AActor* Worker) const
{
	return Worker != nullptr
		&& IsWorkAvailable()
		&& ActiveWorkers.Num() < MaxActiveWorkers
		&& !ActiveWorkers.Contains(Worker)
		&& IsWorkerInRange(Worker);
}

bool UPW_WorkBuildingComponent::BeginWork(AActor* Worker)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || !CanBeginWork(Worker))
	{
		return false;
	}

	ActiveWorkers.AddUnique(Worker);
	Worker->OnDestroyed.AddDynamic(this, &UPW_WorkBuildingComponent::HandleActiveWorkerDestroyed);
	WorkState = EPW_WorkBuildingState::InProgress;
	bHasReservedWork = true;
	RefreshActiveWorkerCount();
	Owner->ForceNetUpdate();
	return true;
}

bool UPW_WorkBuildingComponent::EndWork(AActor* Worker)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || Worker == nullptr)
	{
		return false;
	}

	const bool bRemoved = ActiveWorkers.Remove(Worker) > 0;
	if (bRemoved)
	{
		Worker->OnDestroyed.RemoveDynamic(this, &UPW_WorkBuildingComponent::HandleActiveWorkerDestroyed);
		if (ActiveWorkers.Num() <= 0 && WorkState == EPW_WorkBuildingState::InProgress)
		{
			WorkState = EPW_WorkBuildingState::Reserved;
		}
		RefreshActiveWorkerCount();
		RefreshInteractionGuideProgress();
		Owner->ForceNetUpdate();
	}

	return bRemoved;
}

FTransform UPW_WorkBuildingComponent::GetWorkInteractionTransform(AActor*) const
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return FTransform::Identity;
	}

	const FVector WorkLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * WorkInteractionDistance;
	const FRotator WorkRotation = (-Owner->GetActorForwardVector()).Rotation();
	return FTransform(WorkRotation, WorkLocation);
}

float UPW_WorkBuildingComponent::GetWorkProgressRatio() const
{
	return RequiredWorkProgress > 0.0f ? FMath::Clamp(WorkProgress / RequiredWorkProgress, 0.0f, 1.0f) : 0.0f;
}

float UPW_WorkBuildingComponent::GetRequiredPlayerWorkSeconds() const
{
	return PlayerWorkRate > 0.0f ? RequiredWorkProgress / PlayerWorkRate : 0.0f;
}

void UPW_WorkBuildingComponent::HandleActiveWorkerDestroyed(AActor* DestroyedActor)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || DestroyedActor == nullptr)
	{
		return;
	}

	const bool bRemoved = ActiveWorkers.Remove(DestroyedActor) > 0;
	if (bRemoved)
	{
		if (ActiveWorkers.Num() <= 0 && WorkState == EPW_WorkBuildingState::InProgress)
		{
			WorkState = EPW_WorkBuildingState::Reserved;
		}
		RefreshActiveWorkerCount();
		Owner->ForceNetUpdate();
		RefreshInteractionGuideProgress();
	}
}

void UPW_WorkBuildingComponent::OnRep_WorkGuideState()
{
	RefreshInteractionGuideProgress();
}

void UPW_WorkBuildingComponent::ApplyActiveWorkerProgress(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || WorkState != EPW_WorkBuildingState::InProgress || ActiveWorkers.Num() <= 0 || DeltaTime <= 0.0f)
	{
		return;
	}

	const int32 PreviousWorkerCount = ActiveWorkers.Num();
	RemoveInvalidOrOutOfRangeWorkers();

	if (ActiveWorkers.Num() <= 0)
	{
		WorkState = EPW_WorkBuildingState::Reserved;
		RefreshActiveWorkerCount();
		Owner->ForceNetUpdate();
		return;
	}

	if (ActiveWorkers.Num() != PreviousWorkerCount)
	{
		RefreshActiveWorkerCount();
	}

	WorkProgress = FMath::Min(RequiredWorkProgress, WorkProgress + PlayerWorkRate * DeltaTime * ActiveWorkers.Num());
	RefreshInteractionGuideProgress();
	if (WorkProgress >= RequiredWorkProgress)
	{
		CompleteWork();
		return;
	}

	Owner->ForceNetUpdate();
}

void UPW_WorkBuildingComponent::RemoveInvalidOrOutOfRangeWorkers()
{
	for (int32 WorkerIndex = ActiveWorkers.Num() - 1; WorkerIndex >= 0; --WorkerIndex)
	{
		AActor* Worker = ActiveWorkers[WorkerIndex];
		if (IsValid(Worker) && IsWorkerInRange(Worker))
		{
			continue;
		}

		if (IsValid(Worker))
		{
			Worker->OnDestroyed.RemoveDynamic(this, &UPW_WorkBuildingComponent::HandleActiveWorkerDestroyed);
		}

		ActiveWorkers.RemoveAt(WorkerIndex);
	}
}

void UPW_WorkBuildingComponent::CompleteWork()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	WorkProgress = RequiredWorkProgress;
	WorkState = EPW_WorkBuildingState::Completed;
	bHasReservedWork = false;
	ClearActiveWorkers();
	RefreshInteractionGuideProgress();
	OnWorkCompleted.Broadcast();
	Owner->ForceNetUpdate();
}

void UPW_WorkBuildingComponent::ClearActiveWorkers()
{
	for (AActor* Worker : ActiveWorkers)
	{
		if (IsValid(Worker))
		{
			Worker->OnDestroyed.RemoveDynamic(this, &UPW_WorkBuildingComponent::HandleActiveWorkerDestroyed);
		}
	}

	ActiveWorkers.Reset();
	RefreshActiveWorkerCount();
}

void UPW_WorkBuildingComponent::RefreshActiveWorkerCount()
{
	ActiveWorkerCount = ActiveWorkers.Num();
}

void UPW_WorkBuildingComponent::RefreshInteractionGuideProgress() const
{
	const AActor* Owner = GetOwner();
	UPWInteractableTargetComponent* TargetComponent = Owner != nullptr ? Owner->FindComponentByClass<UPWInteractableTargetComponent>() : nullptr;
	if (TargetComponent == nullptr)
	{
		return;
	}

	TargetComponent->SetInteractionGuideActionProgress(TEXT("Default"), GetWorkProgressRatio());
}

bool UPW_WorkBuildingComponent::IsWorkerInRange(AActor* Worker) const
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || Worker == nullptr)
	{
		return false;
	}

	return FVector::DistSquared(Worker->GetActorLocation(), Owner->GetActorLocation()) <= FMath::Square(WorkInteractionRadius);
}
