#include "Base/PW_WorkBuildingBase.h"

#include "Base/PW_BaseCampActor.h"
#include "Base/PW_BaseCampSubsystem.h"
#include "Base/PW_BaseInventoryAggregatorComponent.h"
#include "Base/PW_BaseWorkTargetRegistryComponent.h"
#include "Base/PW_InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

APW_WorkBuildingBase::APW_WorkBuildingBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetNetCullDistanceSquared(FMath::Square(8000.0f));
	NetUpdateFrequency = 2.0f;

	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	SetRootComponent(BuildingMesh);
	BuildingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BuildingMesh->SetCanEverAffectNavigation(true);

	InternalInventoryComponent = CreateDefaultSubobject<UPW_InventoryComponent>(TEXT("InternalInventoryComponent"));
}

void APW_WorkBuildingBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		RegisterWithBaseCamp();
	}
}

void APW_WorkBuildingBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		UnregisterFromBaseCamp();
	}

	Super::EndPlay(EndPlayReason);
}

void APW_WorkBuildingBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APW_WorkBuildingBase, OwningBaseCamp);
}

bool APW_WorkBuildingBase::TryCraftFromBaseInventory(FName RecipeId)
{
	if (!HasAuthority())
	{
		return false;
	}

	const FPW_WorkRecipe* Recipe = FindRecipe(RecipeId);
	if (Recipe == nullptr || !HasIngredients(*Recipe) || !ConsumeIngredients(*Recipe))
	{
		return false;
	}

	AddCraftResult(*Recipe);
	return true;
}

void APW_WorkBuildingBase::RegisterWithBaseCamp()
{
	UWorld* World = GetWorld();
	UPW_BaseCampSubsystem* BaseCampSubsystem = World != nullptr ? World->GetSubsystem<UPW_BaseCampSubsystem>() : nullptr;
	OwningBaseCamp = BaseCampSubsystem != nullptr ? BaseCampSubsystem->FindBaseCampAtLocation(GetActorLocation()) : nullptr;
	if (OwningBaseCamp != nullptr && OwningBaseCamp->GetWorkTargetRegistryComponent() != nullptr)
	{
		OwningBaseCamp->GetWorkTargetRegistryComponent()->RegisterWorkTarget(this, WorkTargetId, RequiredWorkTag);
	}
}

void APW_WorkBuildingBase::UnregisterFromBaseCamp()
{
	if (OwningBaseCamp != nullptr && OwningBaseCamp->GetWorkTargetRegistryComponent() != nullptr)
	{
		OwningBaseCamp->GetWorkTargetRegistryComponent()->UnregisterWorkTarget(this);
	}

	OwningBaseCamp = nullptr;
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
