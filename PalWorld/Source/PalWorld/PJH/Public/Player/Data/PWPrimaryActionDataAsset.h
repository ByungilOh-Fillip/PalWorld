// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Player/Types/PWPlayerGameplayTypes.h"
#include "PWPrimaryActionDataAsset.generated.h"

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
	float GetDamageVarianceRatio() const { return DamageVarianceRatio; }

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

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Damage", meta = (ClampMin = "0.0", ClampMax = "0.25"))
	float DamageVarianceRatio = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Player|PrimaryAction|Damage", meta = (TitleProperty = "ToolType"))
	TArray<FPWPrimaryActionToolDamage> ToolDamages;
};
