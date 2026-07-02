#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PW_WorkBuildingComponent.generated.h"

UENUM(BlueprintType)
enum class EPW_WorkBuildingState : uint8
{
	Idle,
	Reserved,
	InProgress,
	Completed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPW_WorkBuildingCompletedSignature);

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_WorkBuildingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_WorkBuildingComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	EPW_WorkBuildingState GetWorkState() const { return WorkState; }

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	FName GetWorkId() const { return WorkId; }

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	FGameplayTag GetRequiredWorkTag() const { return RequiredWorkTag; }

	UFUNCTION(BlueprintCallable, Category = "PW|Work")
	void SetHasReservedWork(bool bNewHasReservedWork);

	UFUNCTION(BlueprintCallable, Category = "PW|Work")
	bool ReserveWork(FName NewWorkId);

	UFUNCTION(BlueprintCallable, Category = "PW|Work")
	bool CancelReservedWork();

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	bool HasReservedOrActiveWork() const;

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	bool IsWorkAvailable() const;

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	bool CanBeginWork(AActor* Worker) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Work")
	bool BeginWork(AActor* Worker);

	UFUNCTION(BlueprintCallable, Category = "PW|Work")
	bool EndWork(AActor* Worker);

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	FTransform GetWorkInteractionTransform(AActor* Worker) const;

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	float GetWorkProgressRatio() const;

	UFUNCTION(BlueprintPure, Category = "PW|Work")
	int32 GetActiveWorkerCount() const { return ActiveWorkerCount; }

	UPROPERTY(BlueprintAssignable, Category = "PW|Work")
	FPW_WorkBuildingCompletedSignature OnWorkCompleted;

protected:
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Work")
	FName WorkId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Work", meta = (Categories = "Work"))
	FGameplayTag RequiredWorkTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Work", meta = (ClampMin = "0.0"))
	float RequiredWorkProgress = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Work", meta = (ClampMin = "0.0"))
	float PlayerWorkRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Work", meta = (ClampMin = "0.0"))
	float WorkInteractionDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Work", meta = (ClampMin = "0.0"))
	float WorkInteractionRadius = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Work", meta = (ClampMin = "1"))
	int32 MaxActiveWorkers = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Work")
	bool bStartWithReservedWork = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Work")
	EPW_WorkBuildingState WorkState = EPW_WorkBuildingState::Idle;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Work")
	float WorkProgress = 0.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Work")
	bool bHasReservedWork = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Work")
	int32 ActiveWorkerCount = 0;

private:
	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActiveWorkers;

	UFUNCTION()
	void HandleActiveWorkerDestroyed(AActor* DestroyedActor);

	void ApplyActiveWorkerProgress(float DeltaTime);
	void RemoveInvalidOrOutOfRangeWorkers();
	void CompleteWork();
	void ClearActiveWorkers();
	void RefreshActiveWorkerCount();
	bool IsWorkerInRange(AActor* Worker) const;
};
