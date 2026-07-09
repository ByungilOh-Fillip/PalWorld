// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Player/Types/PWPlayerGameplayTypes.h"
#include "PWPrimaryActionDataAsset.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FPWPrimaryActionToolDamage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PrimaryAction")
	EPWToolType ToolType = EPWToolType::Hand;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PrimaryAction", meta = (ClampMin = "0.0"))
	float TreeDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PrimaryAction", meta = (ClampMin = "0.0"))
	float StoneDamage = 0.f;
};

UCLASS(BlueprintType)
class PALWORLD_API UPWPrimaryActionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPWPrimaryActionDataAsset();

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction")
	float GetTraceDistance() const { return TraceDistance; }

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction")
	float GetMaxViewLocationDistance() const { return MaxViewLocationDistance; }

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction")
	float GetStaminaCost() const { return StaminaCost; }

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction")
	float GetActionDuration() const { return ActionDuration; }

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction")
	float GetActionDurationForTool(EPWToolType ToolType) const;

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction")
	float GetDamageVarianceRatio() const { return DamageVarianceRatio; }

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction|Animation")
	UAnimMontage* GetHarvestMontage() const { return HarvestMontage; }

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction|Animation")
	UAnimMontage* GetHandMontage() const { return HandMontage; }

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction|Animation")
	UAnimMontage* GetPrimaryActionMontage(EPWToolType ToolType) const;

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction|Animation")
	float GetHarvestAnimationBlendInTime() const { return HarvestAnimationBlendInTime; }

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction|Animation")
	float GetHarvestAnimationBlendOutTime() const { return HarvestAnimationBlendOutTime; }

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction|Animation")
	float GetHarvestAnimationPlayRate() const { return HarvestAnimationPlayRate; }

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction|Animation")
	float GetAnimationPlayRate(EPWToolType ToolType) const;

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction|Animation")
	float GetFallbackHitTime(EPWToolType ToolType) const;

	UFUNCTION(BlueprintPure, Category = "Player|PrimaryAction|Animation")
	float GetMinHarvestHitInterval() const { return MinHarvestHitInterval; }

	float GetDamage(EPWToolType ToolType, EPWResourceType ResourceType) const;

private:
	// 애니메이션을 넣기 전까지는 좌클릭 공통 타이밍을 데이터로 관리한다.
	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction", meta = (ClampMin = "50.0"))
	float TraceDistance = 300.f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction", meta = (ClampMin = "100.0"))
	float MaxViewLocationDistance = 800.f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction", meta = (ClampMin = "0.0"))
	float StaminaCost = 8.f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction", meta = (ClampMin = "0.05"))
	float ActionDuration = 0.45f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Hand", meta = (ClampMin = "0.05"))
	float HandActionDuration = 1.73f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Harvest", meta = (ClampMin = "0.05"))
	float HarvestActionDuration = 2.53f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Damage", meta = (ClampMin = "0.0", ClampMax = "0.25"))
	float DamageVarianceRatio = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Damage", meta = (TitleProperty = "ToolType"))
	TArray<FPWPrimaryActionToolDamage> ToolDamages;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Animation")
	TObjectPtr<UAnimMontage> HarvestMontage;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Animation")
	TObjectPtr<UAnimMontage> HandMontage;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Animation", meta = (ClampMin = "0.0"))
	float HarvestAnimationBlendInTime = 0.08f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Animation", meta = (ClampMin = "0.0"))
	float HarvestAnimationBlendOutTime = 0.12f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Animation", meta = (ClampMin = "0.01"))
	float HarvestAnimationPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Animation", meta = (ClampMin = "0.01"))
	float HandAnimationPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Animation", meta = (ClampMin = "0.0"))
	float MinHarvestHitInterval = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Animation", meta = (ClampMin = "0.0"))
	float HarvestFallbackHitTime = 0.45f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Animation", meta = (ClampMin = "0.0"))
	float HandFallbackHitTime = 0.35f;

};
