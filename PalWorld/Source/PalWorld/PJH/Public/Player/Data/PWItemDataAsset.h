// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Player/Types/PWPlayerGameplayTypes.h"
#include "PWItemDataAsset.generated.h"

class USkeletalMesh;
class UStaticMesh;
class UTexture2D;

UCLASS(BlueprintType)
class PALWORLD_API UPWItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Player|Item")
	FName GetItemId() const { return ItemId; }

	UFUNCTION(BlueprintPure, Category = "Player|Item")
	FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintPure, Category = "Player|Item")
	int32 GetMaxStack() const { return MaxStack; }

	UFUNCTION(BlueprintPure, Category = "Player|Item")
	float GetWeight() const { return Weight; }

	UFUNCTION(BlueprintPure, Category = "Player|Item")
	EPWItemType GetItemType() const { return ItemType; }

	UFUNCTION(BlueprintPure, Category = "Player|Item|Visual")
	UTexture2D* GetIcon() const { return Icon; }

	UFUNCTION(BlueprintPure, Category = "Player|Item|Visual")
	UStaticMesh* GetWorldMesh() const { return WorldMesh; }

	UFUNCTION(BlueprintPure, Category = "Player|Item|Equipment")
	bool IsEquippable() const { return bEquippable; }

	UFUNCTION(BlueprintPure, Category = "Player|Item|Equipment")
	EPWEquipmentSlotType GetEquipmentSlotType() const { return EquipmentSlotType; }

	UFUNCTION(BlueprintPure, Category = "Player|Item|Equipment")
	EPWToolType GetToolType() const { return ToolType; }

	UFUNCTION(BlueprintPure, Category = "Player|Item|Equipment")
	UStaticMesh* GetEquipmentStaticMesh() const { return EquipmentStaticMesh; }

	UFUNCTION(BlueprintPure, Category = "Player|Item|Equipment")
	USkeletalMesh* GetEquipmentSkeletalMesh() const { return EquipmentSkeletalMesh; }

	UFUNCTION(BlueprintPure, Category = "Player|Item|Equipment|Attach")
	bool ShouldOverrideHandAttachTransform() const { return bOverrideHandAttachTransform; }

	UFUNCTION(BlueprintPure, Category = "Player|Item|Equipment|Attach")
	FTransform GetHandAttachTransform() const { return HandAttachTransform; }

	UFUNCTION(BlueprintPure, Category = "Player|Item|Equipment|Attach")
	bool ShouldOverrideBackAttachTransform() const { return bOverrideBackAttachTransform; }

	UFUNCTION(BlueprintPure, Category = "Player|Item|Equipment|Attach")
	FTransform GetBackAttachTransform() const { return BackAttachTransform; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item", meta = (AllowPrivateAccess = "true"))
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item", meta = (AllowPrivateAccess = "true"))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 MaxStack = 999;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float Weight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item", meta = (AllowPrivateAccess = "true"))
	EPWItemType ItemType = EPWItemType::Misc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> WorldMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Equipment", meta = (AllowPrivateAccess = "true"))
	bool bEquippable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Equipment", meta = (AllowPrivateAccess = "true", EditCondition = "bEquippable"))
	EPWEquipmentSlotType EquipmentSlotType = EPWEquipmentSlotType::Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Equipment", meta = (AllowPrivateAccess = "true", EditCondition = "bEquippable"))
	EPWToolType ToolType = EPWToolType::Hand;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Equipment", meta = (AllowPrivateAccess = "true", EditCondition = "bEquippable"))
	TObjectPtr<UStaticMesh> EquipmentStaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Equipment", meta = (AllowPrivateAccess = "true", EditCondition = "bEquippable"))
	TObjectPtr<USkeletalMesh> EquipmentSkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Equipment|Attach", meta = (AllowPrivateAccess = "true", EditCondition = "bEquippable"))
	bool bOverrideHandAttachTransform = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Equipment|Attach", meta = (AllowPrivateAccess = "true", EditCondition = "bEquippable && bOverrideHandAttachTransform", EditConditionHides))
	FTransform HandAttachTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Equipment|Attach", meta = (AllowPrivateAccess = "true", EditCondition = "bEquippable"))
	bool bOverrideBackAttachTransform = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Equipment|Attach", meta = (AllowPrivateAccess = "true", EditCondition = "bEquippable && bOverrideBackAttachTransform", EditConditionHides))
	FTransform BackAttachTransform = FTransform::Identity;
};
