#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BaseTypes.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "PW_WorkBuildingBase.generated.h"

class APW_BaseCampActor;
class UStaticMeshComponent;
class UPW_InventoryComponent;

UCLASS(Blueprintable)
class PALWORLD_API APW_WorkBuildingBase : public AActor
{
	GENERATED_BODY()

public:
	APW_WorkBuildingBase();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "PW|Work")
	bool TryCraftFromBaseInventory(FName RecipeId);

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	APW_BaseCampActor* GetOwningBaseCamp() const { return OwningBaseCamp; }

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	UPW_InventoryComponent* GetInternalInventoryComponent() const { return InternalInventoryComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Work")
	TObjectPtr<UStaticMeshComponent> BuildingMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Work")
	TObjectPtr<UPW_InventoryComponent> InternalInventoryComponent;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Work")
	TObjectPtr<APW_BaseCampActor> OwningBaseCamp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Work")
	FName WorkTargetId = TEXT("WorkBuilding");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Work")
	FGameplayTag RequiredWorkTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Work")
	TArray<FPW_WorkRecipe> Recipes;

private:
	void RegisterWithBaseCamp();
	void UnregisterFromBaseCamp();
	const FPW_WorkRecipe* FindRecipe(FName RecipeId) const;
	bool HasIngredients(const FPW_WorkRecipe& Recipe) const;
	bool ConsumeIngredients(const FPW_WorkRecipe& Recipe);
	void AddCraftResult(const FPW_WorkRecipe& Recipe);
};
