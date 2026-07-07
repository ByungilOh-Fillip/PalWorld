#include "Base/PW_WorkBuildingBase.h"

#include "Base/PW_BaseCampActor.h"
#include "Base/PW_BaseCampSubsystem.h"
#include "Base/PW_BaseInventoryAggregatorComponent.h"
#include "Base/PW_BaseWorkTargetRegistryComponent.h"
#include "Base/PW_InventoryComponent.h"
#include "Base/PW_WorkBuildingComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "PWInteractableTargetComponent.h"

APW_WorkBuildingBase::APW_WorkBuildingBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetNetCullDistanceSquared(FMath::Square(8000.0f));
	SetNetUpdateFrequency(2.0f);

	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	SetRootComponent(BuildingMesh);
	BuildingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BuildingMesh->SetCanEverAffectNavigation(true);

	InternalInventoryComponent = CreateDefaultSubobject<UPW_InventoryComponent>(TEXT("InternalInventoryComponent"));
	WorkBuildingComponent = CreateDefaultSubobject<UPW_WorkBuildingComponent>(TEXT("WorkBuildingComponent"));
	InteractableTargetComponent = CreateDefaultSubobject<UPWInteractableTargetComponent>(TEXT("InteractableTargetComponent"));
	InteractableTargetComponent->SetInteractionRadius(250.0f);
	InteractableTargetComponent->SetPromptText(NSLOCTEXT("PWInteraction", "WorkBuildingPrompt", "Use Workbench"));
	InteractableTargetComponent->SetPriority(75);
}

void APW_WorkBuildingBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SynchronizeInteractionGuideActions();
}

void APW_WorkBuildingBase::BeginPlay()
{
	Super::BeginPlay();

	SynchronizeInteractionGuideActions();

	if (HasAuthority())
	{
		RegisterWithBaseCamp();
		if (WorkBuildingComponent != nullptr)
		{
			WorkBuildingComponent->OnWorkCompleted.AddDynamic(this, &APW_WorkBuildingBase::HandleWorkCompleted);
		}
	}
}

void APW_WorkBuildingBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(BaseCampRegistrationRetryTimerHandle);
		}

		if (WorkBuildingComponent != nullptr)
		{
			WorkBuildingComponent->OnWorkCompleted.RemoveDynamic(this, &APW_WorkBuildingBase::HandleWorkCompleted);
		}
		UnregisterFromBaseCamp();
	}

	Super::EndPlay(EndPlayReason);
}

void APW_WorkBuildingBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APW_WorkBuildingBase, OwningBaseCamp);
}

bool APW_WorkBuildingBase::CanInteract_Implementation(AActor* Interactor) const
{
	return Interactor != nullptr && InteractableTargetComponent != nullptr && InteractableTargetComponent->IsInteractionEnabled();
}

bool APW_WorkBuildingBase::Interact_Implementation(AActor* Interactor)
{
	return false;
}

FText APW_WorkBuildingBase::GetInteractionPrompt_Implementation() const
{
	return InteractableTargetComponent ? InteractableTargetComponent->GetPromptText() : NSLOCTEXT("PWInteraction", "WorkBuildingPromptFallback", "Use Workbench");
}

int32 APW_WorkBuildingBase::GetInteractionPriority_Implementation() const
{
	return InteractableTargetComponent ? InteractableTargetComponent->GetPriority() : 75;
}

bool APW_WorkBuildingBase::CanBeginHoldInteraction_Implementation(AActor* Interactor) const
{
	return CanInteract_Implementation(Interactor) && WorkBuildingComponent != nullptr && WorkBuildingComponent->CanBeginWork(Interactor);
}

bool APW_WorkBuildingBase::BeginHoldInteraction_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || WorkBuildingComponent == nullptr || !CanBeginHoldInteraction_Implementation(Interactor))
	{
		return false;
	}

	if (!WorkBuildingComponent->BeginWork(Interactor))
	{
		return false;
	}

	OnWorkerStarted(Interactor);
	ForceNetUpdate();
	return true;
}

void APW_WorkBuildingBase::EndHoldInteraction_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || Interactor == nullptr)
	{
		return;
	}

	if (WorkBuildingComponent != nullptr && WorkBuildingComponent->EndWork(Interactor))
	{
		OnWorkerEnded(Interactor);
		ForceNetUpdate();
	}
}

bool APW_WorkBuildingBase::CanLocalInteract_Implementation(AActor* Interactor) const
{
	return CanInteract_Implementation(Interactor) && !HasReservedOrActiveWork();
}

bool APW_WorkBuildingBase::LocalInteract_Implementation(AActor* Interactor)
{
	if (!CanLocalInteract_Implementation(Interactor))
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[PWInteraction] Workbench UI requested locally. Workbench=%s Interactor=%s"),
		*GetName(),
		*GetNameSafe(Interactor));

	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Green,
			FString::Printf(TEXT("[Interaction] Workbench UI: %s"), *GetName()));
	}

	OnOpenWorkbenchUI(Interactor);
	return true;
}

bool APW_WorkBuildingBase::TryCraftFromBaseInventory(FName RecipeId)
{
	if (!HasAuthority())
	{
		return false;
	}

	const FPW_WorkRecipe* Recipe = FindRecipe(RecipeId);
	if (Recipe == nullptr || WorkBuildingComponent == nullptr || !HasIngredients(*Recipe))
	{
		return false;
	}

	if (!WorkBuildingComponent->ReserveWork(RecipeId))
	{
		return false;
	}

	if (!ConsumeIngredients(*Recipe))
	{
		WorkBuildingComponent->CancelReservedWork();
		return false;
	}

	return true;
}

void APW_WorkBuildingBase::SetHasReservedWork(bool bNewHasReservedWork)
{
	if (!HasAuthority() || WorkBuildingComponent == nullptr)
	{
		return;
	}

	WorkBuildingComponent->SetHasReservedWork(bNewHasReservedWork);
}

bool APW_WorkBuildingBase::HasReservedOrActiveWork() const
{
	return WorkBuildingComponent != nullptr && WorkBuildingComponent->HasReservedOrActiveWork();
}

FTransform APW_WorkBuildingBase::GetWorkInteractionTransform(AActor* Worker) const
{
	return WorkBuildingComponent != nullptr ? WorkBuildingComponent->GetWorkInteractionTransform(Worker) : FTransform::Identity;
}

FGameplayTag APW_WorkBuildingBase::GetRequiredWorkTag() const
{
	return WorkBuildingComponent != nullptr ? WorkBuildingComponent->GetRequiredWorkTag() : FGameplayTag::EmptyTag;
}

void APW_WorkBuildingBase::HandleWorkCompleted()
{
	if (!HasAuthority() || WorkBuildingComponent == nullptr)
	{
		return;
	}

	const FPW_WorkRecipe* Recipe = FindRecipe(WorkBuildingComponent->GetWorkId());
	if (Recipe != nullptr)
	{
		AddCraftResult(*Recipe);
	}
}

void APW_WorkBuildingBase::RegisterWithBaseCamp()
{
	UWorld* World = GetWorld();
	UPW_BaseCampSubsystem* BaseCampSubsystem = World != nullptr ? World->GetSubsystem<UPW_BaseCampSubsystem>() : nullptr;
	OwningBaseCamp = BaseCampSubsystem != nullptr ? BaseCampSubsystem->FindBaseCampAtLocation(GetActorLocation()) : nullptr;
	if (OwningBaseCamp != nullptr && OwningBaseCamp->GetWorkTargetRegistryComponent() != nullptr)
	{
		OwningBaseCamp->GetWorkTargetRegistryComponent()->RegisterWorkTarget(this, WorkTargetId, GetRequiredWorkTag());
		if (World != nullptr)
		{
			World->GetTimerManager().ClearTimer(BaseCampRegistrationRetryTimerHandle);
		}
		return;
	}

	ScheduleBaseCampRegistrationRetry();
}

void APW_WorkBuildingBase::ScheduleBaseCampRegistrationRetry()
{
	UWorld* World = GetWorld();
	if (World == nullptr || World->GetTimerManager().IsTimerActive(BaseCampRegistrationRetryTimerHandle))
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		BaseCampRegistrationRetryTimerHandle,
		this,
		&APW_WorkBuildingBase::RegisterWithBaseCamp,
		0.5f,
		true);
}

void APW_WorkBuildingBase::UnregisterFromBaseCamp()
{
	if (OwningBaseCamp != nullptr && OwningBaseCamp->GetWorkTargetRegistryComponent() != nullptr)
	{
		OwningBaseCamp->GetWorkTargetRegistryComponent()->UnregisterWorkTarget(this);
	}

	OwningBaseCamp = nullptr;
}

void APW_WorkBuildingBase::SynchronizeInteractionGuideActions()
{
	if (WorkBuildingComponent == nullptr || InteractableTargetComponent == nullptr)
	{
		return;
	}

	const float RequiredPlayerWorkSeconds = WorkBuildingComponent->GetRequiredPlayerWorkSeconds();
	if (RequiredPlayerWorkSeconds <= 0.0f)
	{
		return;
	}
	const float WorkProgressRatio = WorkBuildingComponent->GetWorkProgressRatio();

	TArray<FPWInteractionGuideAction> GuideActions;
	InteractableTargetComponent->GetInteractionGuideActions(GuideActions);
	if (GuideActions.Num() <= 0)
	{
		return;
	}

	for (FPWInteractionGuideAction& GuideAction : GuideActions)
	{
		if (GuideAction.ActionId != TEXT("Default") && GuideAction.Key != EKeys::F)
		{
			continue;
		}

		GuideAction.Progress = WorkProgressRatio;
		GuideAction.bShowProgress = true;
		GuideAction.ProgressDurationSeconds = RequiredPlayerWorkSeconds;
	}

	InteractableTargetComponent->SetInteractionGuideActions(GuideActions);
}

const FPW_WorkRecipe* APW_WorkBuildingBase::FindRecipe(FName RecipeId) const
{
	for (const FPW_WorkRecipe& Recipe : Recipes)
	{
		if (Recipe.RecipeId == RecipeId)
		{
			return &Recipe;
		}
	}

	return nullptr;
}

bool APW_WorkBuildingBase::HasIngredients(const FPW_WorkRecipe& Recipe) const
{
	const UPW_BaseInventoryAggregatorComponent* InventoryAggregator = OwningBaseCamp != nullptr
		? OwningBaseCamp->GetInventoryAggregatorComponent()
		: nullptr;
	if (InventoryAggregator == nullptr)
	{
		return false;
	}

	for (const FPW_ItemStack& Ingredient : Recipe.Ingredients)
	{
		if (!Ingredient.IsValid() || InventoryAggregator->GetTotalItemCount(Ingredient.ItemId) < Ingredient.Quantity)
		{
			return false;
		}
	}

	return true;
}

bool APW_WorkBuildingBase::ConsumeIngredients(const FPW_WorkRecipe& Recipe)
{
	UPW_BaseInventoryAggregatorComponent* InventoryAggregator = OwningBaseCamp != nullptr
		? OwningBaseCamp->GetInventoryAggregatorComponent()
		: nullptr;
	if (InventoryAggregator == nullptr)
	{
		return false;
	}

	for (const FPW_ItemStack& Ingredient : Recipe.Ingredients)
	{
		if (!InventoryAggregator->ConsumeItems(Ingredient.ItemId, Ingredient.Quantity))
		{
			return false;
		}
	}

	return true;
}

void APW_WorkBuildingBase::AddCraftResult(const FPW_WorkRecipe& Recipe)
{
	if (!Recipe.ResultItem.IsValid())
	{
		return;
	}

	UPW_BaseInventoryAggregatorComponent* InventoryAggregator = OwningBaseCamp != nullptr
		? OwningBaseCamp->GetInventoryAggregatorComponent()
		: nullptr;
	if (InventoryAggregator == nullptr || !InventoryAggregator->AddItemToAnyStorage(Recipe.ResultItem.ItemId, Recipe.ResultItem.Quantity))
	{
		InternalInventoryComponent->AddItem(Recipe.ResultItem.ItemId, Recipe.ResultItem.Quantity);
	}
}
