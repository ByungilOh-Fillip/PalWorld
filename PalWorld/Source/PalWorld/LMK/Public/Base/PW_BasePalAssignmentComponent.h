#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BaseTypes.h"
#include "Components/ActorComponent.h"
#include "PW_BasePalAssignmentComponent.generated.h"

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_BasePalAssignmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_BasePalAssignmentComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Pal")
	bool AssignPal(FName PalId, TSubclassOf<AActor> PalActorClass);

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Pal")
	bool UnassignPal(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "PW|Base|Pal")
	const TArray<FPW_AssignedPalSlot>& GetAssignedPalSlots() const { return AssignedPalSlots; }

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Pal")
	bool FindIdleAssignedPal(FPW_AssignedPalSlot& OutSlot) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Pal")
	bool TryAssignPalToWorkTarget(int32 SlotIndex, FName WorkTargetId);

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Pal")
	bool SetAssignedPalState(int32 SlotIndex, FName AssignedState);

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Pal")
	bool ClearAssignedPalWorkTarget(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "PW|Base|Pal")
	AActor* GetSpawnedPalActorForSlot(int32 SlotIndex) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Pal", meta = (ClampMin = "0"))
	int32 MaxAssignedPals = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Pal", meta = (ClampMin = "0.0"))
	float SpawnRadius = 600.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Base|Pal")
	TArray<FPW_AssignedPalSlot> AssignedPalSlots;

private:
	int32 FindFreeSlotIndex() const;
	FPW_AssignedPalSlot* FindAssignedPalSlot(int32 SlotIndex);
	const FPW_AssignedPalSlot* FindAssignedPalSlot(int32 SlotIndex) const;
	bool SpawnAssignedPalActor(FPW_AssignedPalSlot& Slot, TSubclassOf<AActor> PalActorClass);
	void DestroyAssignedPalActor(FPW_AssignedPalSlot& Slot);
};
