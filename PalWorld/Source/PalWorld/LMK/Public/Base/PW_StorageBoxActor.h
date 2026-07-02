#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "PWInteractable.h"
#include "PW_StorageBoxActor.generated.h"

class APW_BaseCampActor;
class UStaticMeshComponent;
class UPW_InventoryComponent;
class UPWInteractableTargetComponent;

UCLASS(Blueprintable)
class PALWORLD_API APW_StorageBoxActor : public AActor, public IPWInteractable
{
	GENERATED_BODY()

public:
	APW_StorageBoxActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual bool Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual int32 GetInteractionPriority_Implementation() const override;

	UFUNCTION(BlueprintPure, Category = "PW|Storage")
	UPW_InventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "PW|Storage")
	APW_BaseCampActor* GetOwningBaseCamp() const { return OwningBaseCamp; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Storage")
	TObjectPtr<UStaticMeshComponent> StorageMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Storage")
	TObjectPtr<UPW_InventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Storage|Interaction")
	TObjectPtr<UPWInteractableTargetComponent> InteractableTargetComponent;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Storage")
	TObjectPtr<APW_BaseCampActor> OwningBaseCamp;

private:
	FTimerHandle BaseCampRegistrationRetryTimerHandle;

	void RegisterWithBaseCamp();
	void ScheduleBaseCampRegistrationRetry();
	void UnregisterFromBaseCamp();
};
