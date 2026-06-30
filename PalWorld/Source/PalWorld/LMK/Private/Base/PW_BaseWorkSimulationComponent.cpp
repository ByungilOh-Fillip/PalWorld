#include "Base/PW_BaseWorkSimulationComponent.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UPW_BaseWorkSimulationComponent::UPW_BaseWorkSimulationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPW_BaseWorkSimulationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() != nullptr && GetOwner()->HasAuthority())
	{
		SimulateUntilNow();
	}
}

void UPW_BaseWorkSimulationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPW_BaseWorkSimulationComponent, WorkStates);
}

bool UPW_BaseWorkSimulationComponent::AddWorkState(const FPW_BaseWorkState& WorkState)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || WorkState.WorkId.IsNone())
	{
		return false;
	}

	FPW_BaseWorkState NewState = WorkState;
	if (UWorld* World = GetWorld())
	{
		NewState.LastSimulatedTime = World->GetTimeSeconds();
	}

	WorkStates.Add(NewState);
	Owner->ForceNetUpdate();
	return true;
}

void UPW_BaseWorkSimulationComponent::SimulateUntilNow()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (Owner == nullptr || !Owner->HasAuthority() || World == nullptr)
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	bool bChanged = false;
	for (FPW_BaseWorkState& WorkState : WorkStates)
	{
		if (WorkState.bCompleted)
		{
			WorkState.LastSimulatedTime = CurrentTime;
			continue;
		}

		const double ElapsedTime = FMath::Max(0.0, CurrentTime - WorkState.LastSimulatedTime);
		WorkState.Progress = FMath::Min(WorkState.RequiredProgress, WorkState.Progress + static_cast<float>(ElapsedTime) * WorkState.WorkRate);
		WorkState.LastSimulatedTime = CurrentTime;
		if (WorkState.Progress >= WorkState.RequiredProgress)
		{
			WorkState.bCompleted = true;
		}
		bChanged = true;
	}

	if (bChanged)
	{
		Owner->ForceNetUpdate();
	}
}
