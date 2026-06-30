#include "Base/PW_BasePalAssignmentComponent.h"

#include "Base/PW_BaseCampActor.h"
#include "Base/PW_BaseNavigationComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UPW_BasePalAssignmentComponent::UPW_BasePalAssignmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPW_BasePalAssignmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPW_BasePalAssignmentComponent, AssignedPalSlots);
}

bool UPW_BasePalAssignmentComponent::AssignPal(FName PalId, TSubclassOf<AActor> PalActorClass)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || PalId.IsNone())
	{
		return false;
	}

	const int32 FreeSlotIndex = FindFreeSlotIndex();
	if (FreeSlotIndex == INDEX_NONE)
	{
		return false;
	}

	FPW_AssignedPalSlot NewSlot;
	NewSlot.SlotIndex = FreeSlotIndex;
	NewSlot.PalId = PalId;
	NewSlot.AssignedState = TEXT("Idle");

	SpawnAssignedPalActor(NewSlot, PalActorClass);
	AssignedPalSlots.Add(NewSlot);
	Owner->ForceNetUpdate();
	return true;
}

bool UPW_BasePalAssignmentComponent::UnassignPal(int32 SlotIndex)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return false;
	}

	for (int32 Index = 0; Index < AssignedPalSlots.Num(); ++Index)
	{
		if (AssignedPalSlots[Index].SlotIndex == SlotIndex)
		{
			DestroyAssignedPalActor(AssignedPalSlots[Index]);
			AssignedPalSlots.RemoveAt(Index);
			Owner->ForceNetUpdate();
			return true;
		}
	}

	return false;
}

int32 UPW_BasePalAssignmentComponent::FindFreeSlotIndex() const
{
	for (int32 CandidateIndex = 0; CandidateIndex < MaxAssignedPals; ++CandidateIndex)
	{
		bool bUsed = false;
		for (const FPW_AssignedPalSlot& Slot : AssignedPalSlots)
		{
			if (Slot.SlotIndex == CandidateIndex)
			{
				bUsed = true;
				break;
			}
		}

		if (!bUsed)
		{
			return CandidateIndex;
		}
	}

	return INDEX_NONE;
}

bool UPW_BasePalAssignmentComponent::SpawnAssignedPalActor(FPW_AssignedPalSlot& Slot, TSubclassOf<AActor> PalActorClass)
{
	AActor* Owner = GetOwner();
	UWorld* World = Owner != nullptr ? Owner->GetWorld() : nullptr;
	if (World == nullptr || PalActorClass == nullptr)
	{
		return false;
	}

	FVector SpawnLocation = Owner->GetActorLocation();
	const APW_BaseCampActor* BaseCampActor = Cast<APW_BaseCampActor>(Owner);
	const UPW_BaseNavigationComponent* BaseNavigationComponent = BaseCampActor != nullptr ? BaseCampActor->GetBaseNavigationComponent() : nullptr;
	if (BaseNavigationComponent != nullptr)
	{
		if (!BaseNavigationComponent->IsBaseNavigationActive())
		{
			return false;
		}

		if (!BaseNavigationComponent->GetRandomReachablePointInBase(Owner->GetActorLocation(), SpawnRadius, SpawnLocation))
		{
			return false;
		}
	}
	else
	{
		const FVector Offset = FVector(FMath::FRandRange(-SpawnRadius, SpawnRadius), FMath::FRandRange(-SpawnRadius, SpawnRadius), 0.0f);
		SpawnLocation += Offset;
	}

	const FTransform SpawnTransform(Owner->GetActorRotation(), SpawnLocation);
	AActor* SpawnedActor = World->SpawnActor<AActor>(PalActorClass, SpawnTransform);
	if (SpawnedActor != nullptr)
	{
		Slot.SpawnedPalActor = SpawnedActor;
		return true;
	}

	return false;
}

void UPW_BasePalAssignmentComponent::DestroyAssignedPalActor(FPW_AssignedPalSlot& Slot)
{
	if (IsValid(Slot.SpawnedPalActor))
	{
		Slot.SpawnedPalActor->Destroy();
	}

	Slot.SpawnedPalActor = nullptr;
}
