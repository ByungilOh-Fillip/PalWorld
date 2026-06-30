// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PWItemDataAsset.generated.h"

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

	UFUNCTION(BlueprintPure, Category = "Player|Item|Visual")
	UTexture2D* GetIcon() const { return Icon; }

	UFUNCTION(BlueprintPure, Category = "Player|Item|Visual")
	UStaticMesh* GetWorldMesh() const { return WorldMesh; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item", meta = (AllowPrivateAccess = "true"))
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item", meta = (AllowPrivateAccess = "true"))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 MaxStack = 999;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float Weight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Item|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> WorldMesh = nullptr;
};
