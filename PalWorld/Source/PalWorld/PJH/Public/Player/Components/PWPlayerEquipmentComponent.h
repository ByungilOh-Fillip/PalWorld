// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player/Types/PWPlayerGameplayTypes.h"
#include "PWPlayerEquipmentComponent.generated.h"

class APWPlayerCharacter;
class USceneComponent;
class USkeletalMeshComponent;
class USkeletalMesh;
class UStaticMesh;

USTRUCT(BlueprintType)
struct FPWEquipmentSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	EPWToolType ToolType = EPWToolType::Hand;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<USkeletalMesh> SkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FTransform HandAttachTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FTransform BackAttachTransform = FTransform::Identity;

	bool HasVisualMesh() const
	{
		return StaticMesh || SkeletalMesh;
	}
};

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerEquipmentComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Player|Equipment")
	void SelectEquipmentSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Player|Equipment")
	void SelectNextEquipmentSlot();

	UFUNCTION(BlueprintCallable, Category = "Player|Equipment")
	void SelectPreviousEquipmentSlot();

	UFUNCTION(BlueprintPure, Category = "Player|Equipment")
	int32 GetSelectedSlotIndex() const { return SelectedSlotIndex; }

	UFUNCTION(BlueprintPure, Category = "Player|Equipment")
	EPWToolType GetSelectedToolType() const;

	UFUNCTION(BlueprintPure, Category = "Player|Equipment")
	bool IsSlotEnabled(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Player|Equipment")
	const FPWEquipmentSlotData& GetSlotData(int32 SlotIndex) const;

	static constexpr int32 EquipmentSlotCount = 4;

private:
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_EquipmentSlots, Category = "Player|Equipment", meta = (TitleProperty = "ToolType"))
	TArray<FPWEquipmentSlotData> EquipmentSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Equipment|Sockets")
	FName HandSocketName = TEXT("hand_r");

	UPROPERTY(EditDefaultsOnly, Category = "Player|Equipment|Sockets")
	FName BackLeftSocketName = TEXT("spine_03");

	UPROPERTY(EditDefaultsOnly, Category = "Player|Equipment|Sockets")
	FName BackRightSocketName = TEXT("spine_03");

	UPROPERTY(ReplicatedUsing = OnRep_SelectedSlotIndex, VisibleInstanceOnly, Category = "Player|Equipment")
	int32 SelectedSlotIndex = 0;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USceneComponent>> SlotVisualComponents;

	UFUNCTION(Server, Reliable)
	void ServerSelectEquipmentSlot(int32 NewSlotIndex);

	UFUNCTION()
	void OnRep_EquipmentSlots();

	UFUNCTION()
	void OnRep_SelectedSlotIndex();

	APWPlayerCharacter* GetPlayerCharacter() const;
	USkeletalMeshComponent* GetCharacterMesh() const;

	void EnsureSlotCount();
	bool IsValidSlotIndex(int32 SlotIndex) const;
	bool IsSlotSelectable(int32 SlotIndex) const;
	int32 FindSelectableSlotByOffset(int32 StartSlotIndex, int32 Offset) const;
	void SetSelectedSlotIndex(int32 NewSlotIndex);
	void SyncSelectedToolToGatherComponent() const;

	void RebuildVisualComponents();
	void DestroyVisualComponents();
	USceneComponent* CreateVisualComponentForSlot(int32 SlotIndex);
	void UpdateEquipmentVisuals();
	void AttachSlotVisual(int32 SlotIndex, FName SocketName, const FTransform& RelativeTransform);
	void HideSlotVisual(int32 SlotIndex);
	FName GetBackSocketNameForSlot(int32 SlotIndex) const;
	bool ShouldShowSlotOnBack(int32 SlotIndex) const;
};
