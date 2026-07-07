#include "Base/PW_BaseCampActor.h"

#include "Base/PW_BaseCampSubsystem.h"
#include "Base/PW_BaseInventoryAggregatorComponent.h"
#include "Base/PW_BaseNavigationComponent.h"
#include "Base/PW_BaseOwnershipComponent.h"
#include "Base/PW_BasePalAssignmentComponent.h"
#include "Base/PW_BaseWorkSimulationComponent.h"
#include "Base/PW_BaseWorkTargetRegistryComponent.h"
#include "Base/PW_WorkBuildingComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "NavigationInvokerComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "PWInteractableTargetComponent.h"
#include "PWSkillComponent.h"

APW_BaseCampActor::APW_BaseCampActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetNetCullDistanceSquared(FMath::Square(12000.0f));
	SetNetUpdateFrequency(2.0f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	OwnershipComponent = CreateDefaultSubobject<UPW_BaseOwnershipComponent>(TEXT("OwnershipComponent"));
	InventoryAggregatorComponent = CreateDefaultSubobject<UPW_BaseInventoryAggregatorComponent>(TEXT("InventoryAggregatorComponent"));
	PalAssignmentComponent = CreateDefaultSubobject<UPW_BasePalAssignmentComponent>(TEXT("PalAssignmentComponent"));
	WorkTargetRegistryComponent = CreateDefaultSubobject<UPW_BaseWorkTargetRegistryComponent>(TEXT("WorkTargetRegistryComponent"));
	WorkSimulationComponent = CreateDefaultSubobject<UPW_BaseWorkSimulationComponent>(TEXT("WorkSimulationComponent"));
	BaseNavigationComponent = CreateDefaultSubobject<UPW_BaseNavigationComponent>(TEXT("BaseNavigationComponent"));
	NavigationInvokerComponent = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavigationInvokerComponent"));
	InteractableTargetComponent = CreateDefaultSubobject<UPWInteractableTargetComponent>(TEXT("InteractableTargetComponent"));
	InteractableTargetComponent->SetInteractionRadius(350.0f);
	InteractableTargetComponent->SetPromptText(NSLOCTEXT("PWInteraction", "BaseCampPrompt", "Open Base Camp"));
	InteractableTargetComponent->SetPriority(50);
}

void APW_BaseCampActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && !BaseCampId.IsValid())
	{
		BaseCampId.GenerateNewId();
	}

	DrawDebugCampRadius();
	if (BaseNavigationComponent != nullptr)
	{
		BaseNavigationComponent->ConfigureNavigationInvoker(CampRadius);
		if (bDrawDebugNavigation)
		{
			BaseNavigationComponent->DrawDebugNavigationSample();
		}
	}

	RegisterWithSubsystem();

	if (HasAuthority())
	{
		StartVisitorChecks();
	}
}

void APW_BaseCampActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (BaseNavigationComponent != nullptr)
	{
		BaseNavigationComponent->ConfigureNavigationInvoker(CampRadius);
	}
}

void APW_BaseCampActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		StopVisitorChecks();
	}

	UnregisterFromSubsystem();

	Super::EndPlay(EndPlayReason);
}

void APW_BaseCampActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APW_BaseCampActor, BaseCampId);
	DOREPLIFETIME(APW_BaseCampActor, OwnerId);
	DOREPLIFETIME(APW_BaseCampActor, CampRadius);
	DOREPLIFETIME(APW_BaseCampActor, CampLevel);
}

void APW_BaseCampActor::SetBaseOwnerId(const FPW_BaseOwnerId& NewOwnerId)
{
	if (HasAuthority())
	{
		OwnerId = NewOwnerId;
		ForceNetUpdate();
	}
}

bool APW_BaseCampActor::CanInteract_Implementation(AActor* Interactor) const
{
	return Interactor != nullptr && InteractableTargetComponent != nullptr && InteractableTargetComponent->IsInteractionEnabled();
}

bool APW_BaseCampActor::Interact_Implementation(AActor* Interactor)
{
	if (!CanInteract_Implementation(Interactor))
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[PWInteraction] Base camp interacted. Base=%s Interactor=%s"),
		*GetName(),
		*GetNameSafe(Interactor));

	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Green,
			FString::Printf(TEXT("[Interaction] BaseCamp: %s"), *GetName()));
	}

	return true;
}

FText APW_BaseCampActor::GetInteractionPrompt_Implementation() const
{
	return InteractableTargetComponent ? InteractableTargetComponent->GetPromptText() : NSLOCTEXT("PWInteraction", "BaseCampPromptFallback", "Open Base Camp");
}

int32 APW_BaseCampActor::GetInteractionPriority_Implementation() const
{
	return InteractableTargetComponent ? InteractableTargetComponent->GetPriority() : 50;
}

bool APW_BaseCampActor::ContainsLocation(const FVector& Location) const
{
	return FVector::DistSquared2D(GetActorLocation(), Location) <= FMath::Square(CampRadius);
}

bool APW_BaseCampActor::TryAssignIdlePalToWorkTargetByTag(FGameplayTag RequiredWorkTag, FPW_AssignedPalSlot& OutAssignedSlot, FPW_WorkTargetEntry& OutWorkTarget)
{
	if (!HasAuthority() || PalAssignmentComponent == nullptr || WorkTargetRegistryComponent == nullptr || !RequiredWorkTag.IsValid())
	{
		return false;
	}

	TArray<FPW_WorkTargetEntry> CandidateTargets;
	WorkTargetRegistryComponent->GetWorkTargetsByTag(RequiredWorkTag, CandidateTargets);
	if (CandidateTargets.Num() <= 0)
	{
		return false;
	}

	for (const FPW_AssignedPalSlot& Slot : PalAssignmentComponent->GetAssignedPalSlots())
	{
		AActor* PalActor = Slot.SpawnedPalActor.Get();
		const UPWSkillComponent* SkillComponent = PalActor != nullptr ? PalActor->FindComponentByClass<UPWSkillComponent>() : nullptr;
		if (Slot.AssignedState != TEXT("Idle") || !Slot.CurrentWorkTargetId.IsNone() || SkillComponent == nullptr || !SkillComponent->CanWork(RequiredWorkTag))
		{
			continue;
		}

		for (const FPW_WorkTargetEntry& CandidateTarget : CandidateTargets)
		{
			const AActor* TargetActor = CandidateTarget.TargetActor.Get();
			const UPW_WorkBuildingComponent* WorkBuildingComponent = TargetActor != nullptr ? TargetActor->FindComponentByClass<UPW_WorkBuildingComponent>() : nullptr;
			if (WorkBuildingComponent == nullptr || !WorkBuildingComponent->IsWorkAvailable())
			{
				continue;
			}

			if (PalAssignmentComponent->TryAssignPalToWorkTarget(Slot.SlotIndex, CandidateTarget.WorkTargetId))
			{
				OutAssignedSlot = Slot;
				OutAssignedSlot.CurrentWorkTargetId = CandidateTarget.WorkTargetId;
				OutAssignedSlot.AssignedState = TEXT("MovingToWork");
				OutWorkTarget = CandidateTarget;
				ForceNetUpdate();
				return true;
			}
		}
	}

	return false;
}

void APW_BaseCampActor::RegisterWithSubsystem()
{
	if (UWorld* World = GetWorld())
	{
		if (UPW_BaseCampSubsystem* BaseCampSubsystem = World->GetSubsystem<UPW_BaseCampSubsystem>())
		{
			BaseCampSubsystem->RegisterBaseCamp(this);
		}
	}
}

void APW_BaseCampActor::UnregisterFromSubsystem()
{
	if (UWorld* World = GetWorld())
	{
		if (UPW_BaseCampSubsystem* BaseCampSubsystem = World->GetSubsystem<UPW_BaseCampSubsystem>())
		{
			BaseCampSubsystem->UnregisterBaseCamp(this);
		}
	}
}

void APW_BaseCampActor::DrawDebugCampRadius() const
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!bDrawDebugRadius)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (World == nullptr || World->IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	DrawDebugSphere(
		World,
		GetActorLocation(),
		CampRadius,
		64,
		FColor::Blue,
		true,
		-1.0f,
		0,
		5.0f);
#endif
}

void APW_BaseCampActor::StartVisitorChecks()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	LastVisitedServerTime = World->GetTimeSeconds();
	EvaluateVisitorActivity();

	World->GetTimerManager().SetTimer(
		VisitorCheckTimerHandle,
		this,
		&APW_BaseCampActor::EvaluateVisitorActivity,
		VisitorCheckIntervalSeconds,
		true);
}

void APW_BaseCampActor::StopVisitorChecks()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VisitorCheckTimerHandle);
	}
}

void APW_BaseCampActor::EvaluateVisitorActivity()
{
	UWorld* World = GetWorld();
	if (World == nullptr || !HasAuthority())
	{
		return;
	}

	const bool bVisitorNearby = IsAnyPlayerWithinActivationRadius();
	const double CurrentTime = World->GetTimeSeconds();
	if (bVisitorNearby)
	{
		LastVisitedServerTime = CurrentTime;
		SetBaseActiveState(true);
		return;
	}

	if (CurrentTime - LastVisitedServerTime >= VisitorKeepAliveSeconds)
	{
		SetBaseActiveState(false);
	}
}

bool APW_BaseCampActor::IsAnyPlayerWithinActivationRadius() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	const float ActivationRadiusSquared = FMath::Square(CampRadius + VisitorActivationExtraRadius);
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		const APawn* Pawn = PlayerController != nullptr ? PlayerController->GetPawn() : nullptr;
		if (Pawn != nullptr && FVector::DistSquared2D(GetActorLocation(), Pawn->GetActorLocation()) <= ActivationRadiusSquared)
		{
			return true;
		}
	}

	return false;
}

void APW_BaseCampActor::SetBaseActiveState(bool bNewHasActiveVisitor)
{
	if (bHasActiveVisitor == bNewHasActiveVisitor)
	{
		return;
	}

	if (WorkSimulationComponent != nullptr)
	{
		WorkSimulationComponent->SimulateUntilNow();
	}

	bHasActiveVisitor = bNewHasActiveVisitor;
	if (BaseNavigationComponent != nullptr)
	{
		BaseNavigationComponent->SetBaseNavigationActive(bHasActiveVisitor);
	}
}
