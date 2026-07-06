// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PWWorldItemDropLibrary.generated.h"

class APWWorldItemActor;
class UPWItemDataAsset;

USTRUCT(BlueprintType)
struct FPWWorldItemDropRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop")
	TObjectPtr<UPWItemDataAsset> ItemData = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop")
	FName ItemId = NAME_None;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop", meta = (ClampMin = "1"))
	int32 Count = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop")
	TObjectPtr<AActor> PreferredReceiver = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop")
	FVector SourceLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TowardTargetMinAlpha = 0.55f;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TowardTargetMaxAlpha = 0.85f;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float ScatterRadius = 60.f;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop")
	bool bIgnoreSourceActorInGroundTrace = false;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop")
	TSubclassOf<APWWorldItemActor> WorldItemActorClass;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float MinHorizontalImpulse = 130.f;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float MaxHorizontalImpulse = 230.f;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float MinUpwardImpulse = 190.f;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float MaxUpwardImpulse = 290.f;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop")
	bool bStartAutoCollect = false;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float AutoCollectMinDelay = 0.5f;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float AutoCollectMaxDelay = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Player|World Item|Drop", meta = (ClampMin = "0.0"))
	float AutoCollectRadius = 900.f;
};

UCLASS()
class PALWORLD_API UPWWorldItemDropLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|World Item|Drop", meta = (WorldContext = "WorldContextObject"))
	static APWWorldItemActor* SpawnWorldItemDrop(UObject* WorldContextObject, const FPWWorldItemDropRequest& Request);

	UFUNCTION(BlueprintPure, Category = "Player|World Item|Drop")
	static bool CanSpawnWorldItem(UPWItemDataAsset* ItemData);
};
