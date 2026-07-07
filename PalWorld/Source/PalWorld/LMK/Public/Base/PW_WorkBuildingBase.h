#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BaseTypes.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "PWHoldInteractable.h"
#include "PWInteractable.h"
#include "PWLocalInteractable.h"
#include "PW_WorkBuildingBase.generated.h"

class APW_BaseCampActor;
class UStaticMeshComponent;
class UPW_InventoryComponent;
class UPW_WorkBuildingComponent;
class UPWInteractableTargetComponent;

UCLASS(Blueprintable)
class PALWORLD_API APW_WorkBuildingBase : public AActor, public IPWInteractable, public IPWHoldInteractable, public IPWLocalInteractable
{
	GENERATED_BODY()

public:
	APW_WorkBuildingBase();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual bool Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual int32 GetInteractionPriority_Implementation() const override;
	virtual bool CanBeginHoldInteraction_Implementation(AActor* Interactor) const override;
	virtual bool BeginHoldInteraction_Implementation(AActor* Interactor) override;
	virtual void EndHoldInteraction_Implementation(AActor* Interactor) override;
	virtual bool CanLocalInteract_Implementation(AActor* Interactor) const override;
	virtual bool LocalInteract_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable, Category = "PW|Work")
	bool TryCraftFromBaseInventory(FName RecipeId);

	UFUNCTION(BlueprintCallable, Category = "PW|Work")
	void SetHasReservedWork(bool bNewHasReservedWork);

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	bool HasReservedOrActiveWork() const;

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	FTransform GetWorkInteractionTransform(AActor* Worker) const;

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	FGameplayTag GetRequiredWorkTag() const;

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	APW_BaseCampActor* GetOwningBaseCamp() const { return OwningBaseCamp; }

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	UPW_InventoryComponent* GetInternalInventoryComponent() const { return InternalInventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	UPW_WorkBuildingComponent* GetWorkBuildingComponent() const { return WorkBuildingComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Components")
	TObjectPtr<UStaticMeshComponent> BuildingMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Components")
	TObjectPtr<UPW_InventoryComponent> InternalInventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Components")
	TObjectPtr<UPW_WorkBuildingComponent> WorkBuildingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Components")
	TObjectPtr<UPWInteractableTargetComponent> InteractableTargetComponent;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Work")
	TObjectPtr<APW_BaseCampActor> OwningBaseCamp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Work")
	FName WorkTargetId = TEXT("WorkBuilding");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Work")
	TArray<FPW_WorkRecipe> Recipes;

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Work")
	void OnOpenWorkbenchUI(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Work")
	void OnWorkerStarted(AActor* Worker);

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Work")
	void OnWorkerEnded(AActor* Worker);

private:
	FTimerHandle BaseCampRegistrationRetryTimerHandle;

	UFUNCTION()
	void HandleWorkCompleted();

	void RegisterWithBaseCamp();
	void ScheduleBaseCampRegistrationRetry();
	void UnregisterFromBaseCamp();
	void SynchronizeInteractionGuideActions();
	const FPW_WorkRecipe* FindRecipe(FName RecipeId) const;
	bool HasIngredients(const FPW_WorkRecipe& Recipe) const;
	bool ConsumeIngredients(const FPW_WorkRecipe& Recipe);
	void AddCraftResult(const FPW_WorkRecipe& Recipe);
};
