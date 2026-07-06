// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PWInteractable.h"
#include "PWWorldItemActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UPWInteractableTargetComponent;
class UPWItemDataAsset;
class UMaterialInterface;

UCLASS()
class PALWORLD_API APWWorldItemActor : public AActor, public IPWInteractable
{
	GENERATED_BODY()

public:
	APWWorldItemActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeWorldItem(UPWItemDataAsset* InItemData, FName InItemId, int32 InCount, AActor* InPreferredReceiver, const FVector& InDropImpulse = FVector::ZeroVector);

	static FVector ResolveGroundedDropLocation(
		UWorld* World,
		const FVector& DesiredLocation,
		const TArray<AActor*>& IgnoredActors,
		float TraceUpDistance = 180.f,
		float TraceDownDistance = 650.f,
		float GroundOffset = 28.f);

	bool Collect(AActor* Collector);
	void SetHighlighted(bool bHighlighted);
	void StartAutoCollect(float InitialDelay, float InAutoCollectRadius);

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual bool Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual int32 GetInteractionPriority_Implementation() const override;

	UFUNCTION(BlueprintPure, Category = "Player|World Item")
	FName GetItemId() const { return ItemId; }

	UFUNCTION(BlueprintPure, Category = "Player|World Item")
	int32 GetCount() const { return Count; }

	UFUNCTION(BlueprintPure, Category = "Player|World Item")
	UPWItemDataAsset* GetItemData() const { return ItemData; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|World Item")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|World Item")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|World Item|Visual")
	TObjectPtr<UStaticMeshComponent> OutlineMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|World Item|Interaction")
	TObjectPtr<UPWInteractableTargetComponent> InteractableTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|World Item|Interaction", meta = (ClampMin = "0.0"))
	float PickupRadius = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|World Item|Visual")
	int32 HighlightStencilValue = 252;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|World Item|Visual", meta = (ClampMin = "1.0"))
	float OutlineScale = 1.06f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|World Item|Visual")
	TObjectPtr<UMaterialInterface> OutlineMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|World Item|Drop")
	bool bUseDropPhysics = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float DropSettleDelay = 1.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float MinRandomHorizontalImpulse = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float MaxRandomHorizontalImpulse = 170.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float MinUpwardImpulse = 160.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float MaxUpwardImpulse = 260.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float AngularImpulseStrength = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float DroppedItemLifeSeconds = 300.f;

private:
	UPROPERTY(ReplicatedUsing = OnRep_ItemData)
	TObjectPtr<UPWItemDataAsset> ItemData = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_ItemState)
	FName ItemId = NAME_None;

	UPROPERTY(ReplicatedUsing = OnRep_ItemState)
	int32 Count = 0;

	UPROPERTY(Replicated)
	TObjectPtr<AActor> PreferredReceiver = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_DropVisualTransform)
	FTransform ReplicatedDropVisualTransform = FTransform::Identity;

	UPROPERTY(ReplicatedUsing = OnRep_DropVisualTransform)
	bool bHasReplicatedDropVisualTransform = false;

	FTimerHandle AutoCollectTimerHandle;
	FTimerHandle DropSettleTimerHandle;
	float AutoCollectRadius = 350.f;
	bool bDropPhysicsActive = false;

	UFUNCTION()
	void OnRep_ItemData();

	UFUNCTION()
	void OnRep_ItemState();

	UFUNCTION()
	void OnRep_DropVisualTransform();

	FVector GetItemWorldLocation() const;
	void RefreshVisuals();
	void ApplyReplicatedDropVisualTransform();
	void UpdateReplicatedDropVisualTransform();
	void StartDropPhysics(const FVector& InDropImpulse);
	void SettleDropPhysics();
	AActor* ResolveReceiver(AActor* Collector) const;
	void TryAutoCollect();
};
