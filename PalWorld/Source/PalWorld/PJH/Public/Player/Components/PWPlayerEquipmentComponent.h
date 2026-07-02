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
class UPWItemDataAsset;
class UPWPlayerInventoryLinkComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPWEquipmentChangedSignature);

USTRUCT(BlueprintType)
struct FPWEquipmentSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	EPWEquipmentSlotType SlotType = EPWEquipmentSlotType::Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UPWItemDataAsset> ItemData = nullptr;

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
		return StaticMesh || SkeletalMesh || GetEquipmentStaticMesh() || GetEquipmentSkeletalMesh();
	}

	EPWToolType GetToolType() const;
	EPWEquipmentSlotType GetEquipmentSlotType() const;
	UStaticMesh* GetEquipmentStaticMesh() const;
	USkeletalMesh* GetEquipmentSkeletalMesh() const;
	FTransform GetHandAttachTransform() const;
	FTransform GetBackAttachTransform() const;
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
	int32 GetEquipmentSlotCount() const { return EquipmentSlotCount; }

	UFUNCTION(BlueprintPure, Category = "Player|Equipment")
	EPWToolType GetSelectedToolType() const;

	UFUNCTION(BlueprintPure, Category = "Player|Equipment")
	bool IsSlotEnabled(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Player|Equipment")
	const FPWEquipmentSlotData& GetSlotData(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Player|Equipment")
	EPWEquipmentSlotType GetSlotType(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Player|Equipment")
	bool IsWeaponQuickSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Player|Equipment")
	bool CanEquipItemToSlot(UPWItemDataAsset* ItemData, int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Player|Equipment")
	bool SetEquipmentItem(int32 SlotIndex, UPWItemDataAsset* ItemData);

	UFUNCTION(BlueprintCallable, Category = "Player|Equipment")
	bool ClearEquipmentSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Player|Equipment")
	bool MoveEquipmentSlot(int32 FromSlotIndex, int32 ToSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Player|Equipment")
	bool EquipFromInventorySlot(int32 InventorySlotIndex, int32 EquipmentSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Player|Equipment")
	bool EquipFromInventorySlotToFirstAvailable(int32 InventorySlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Player|Equipment")
	bool UnequipToInventory(int32 EquipmentSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Player|Equipment")
	bool UnequipToInventorySlot(int32 EquipmentSlotIndex, int32 InventorySlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Player|Equipment")
	bool DropEquipmentSlot(int32 EquipmentSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Player|Equipment")
	bool DestroyEquipmentSlot(int32 EquipmentSlotIndex);

	UPROPERTY(BlueprintAssignable, Category = "Player|Equipment")
	FPWEquipmentChangedSignature OnEquipmentChanged;

	static constexpr int32 WeaponSlotCount = 4;
	static constexpr int32 HeadSlotIndex = 4;
	static constexpr int32 BodySlotIndex = 5;
	static constexpr int32 ShieldSlotIndex = 6;
	static constexpr int32 GliderSlotIndex = 7;
	static constexpr int32 SphereModuleSlotIndex = 8;
	static constexpr int32 AccessorySlotStartIndex = 9;
	static constexpr int32 AccessorySlotCount = 2;
	static constexpr int32 FoodSlotStartIndex = 11;
	static constexpr int32 FoodSlotCount = 4;
	static constexpr int32 EquipmentSlotCount = FoodSlotStartIndex + FoodSlotCount;

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

	UFUNCTION(Server, Reliable)
	void ServerSetEquipmentItem(int32 SlotIndex, UPWItemDataAsset* ItemData);

	UFUNCTION(Server, Reliable)
	void ServerClearEquipmentSlot(int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerMoveEquipmentSlot(int32 FromSlotIndex, int32 ToSlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerEquipFromInventorySlot(int32 InventorySlotIndex, int32 EquipmentSlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerEquipFromInventorySlotToFirstAvailable(int32 InventorySlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerUnequipToInventory(int32 EquipmentSlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerUnequipToInventorySlot(int32 EquipmentSlotIndex, int32 InventorySlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerDropEquipmentSlot(int32 EquipmentSlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerDestroyEquipmentSlot(int32 EquipmentSlotIndex);

	UFUNCTION()
	void OnRep_EquipmentSlots();

	UFUNCTION()
	void OnRep_SelectedSlotIndex();

	APWPlayerCharacter* GetPlayerCharacter() const;
	USkeletalMeshComponent* GetCharacterMesh() const;
	UPWPlayerInventoryLinkComponent* GetInventoryComponent() const;

	void EnsureSlotCount();
	void ApplyDefaultSlotTypes();
	bool IsValidSlotIndex(int32 SlotIndex) const;
	bool IsSlotSelectable(int32 SlotIndex) const;
	bool IsItemCompatibleWithSlot(UPWItemDataAsset* ItemData, int32 SlotIndex) const;
	bool CanSwapEquipmentSlots(int32 FromSlotIndex, int32 ToSlotIndex) const;
	int32 FindSelectableSlotByOffset(int32 StartSlotIndex, int32 Offset) const;
	void SetSelectedSlotIndex(int32 NewSlotIndex);
	void SyncSelectedToolToPrimaryActionComponent() const;
	bool SetEquipmentItemAuthority(int32 SlotIndex, UPWItemDataAsset* ItemData);
	bool ClearEquipmentSlotAuthority(int32 SlotIndex);
	bool MoveEquipmentSlotAuthority(int32 FromSlotIndex, int32 ToSlotIndex);
	bool EquipFromInventorySlotAuthority(int32 InventorySlotIndex, int32 EquipmentSlotIndex);
	bool EquipFromInventorySlotToFirstAvailableAuthority(int32 InventorySlotIndex);
	bool UnequipToInventoryAuthority(int32 EquipmentSlotIndex);
	bool UnequipToInventorySlotAuthority(int32 EquipmentSlotIndex, int32 InventorySlotIndex);
	int32 FindFirstCompatibleEquipmentSlotIndex(UPWItemDataAsset* ItemData) const;
	void NotifyEquipmentChanged();

	void RebuildVisualComponents();
	void DestroyVisualComponents();
	USceneComponent* CreateVisualComponentForSlot(int32 SlotIndex);
	void UpdateEquipmentVisuals();
	void AttachSlotVisual(int32 SlotIndex, FName SocketName, const FTransform& RelativeTransform);
	void HideSlotVisual(int32 SlotIndex);
	FName GetBackSocketNameForSlot(int32 SlotIndex) const;
	bool ShouldShowSlotOnBack(int32 SlotIndex) const;
};
