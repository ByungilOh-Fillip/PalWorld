#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PW_BaseTypes.generated.h"

UENUM(BlueprintType)
enum class EPW_BaseOwnerType : uint8
{
	Player,
	Guild
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_BaseCampId
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base")
	FGuid Value;

	bool IsValid() const { return Value.IsValid(); }
	void GenerateNewId() { Value = FGuid::NewGuid(); }

	bool operator==(const FPW_BaseCampId& Other) const
	{
		return Value == Other.Value;
	}
};

FORCEINLINE uint32 GetTypeHash(const FPW_BaseCampId& BaseCampId)
{
	return GetTypeHash(BaseCampId.Value);
}

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_BaseOwnerId
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base")
	EPW_BaseOwnerType OwnerType = EPW_BaseOwnerType::Player;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base")
	FString OwnerId;

	bool IsValid() const { return !OwnerId.IsEmpty(); }

	bool operator==(const FPW_BaseOwnerId& Other) const
	{
		return OwnerType == Other.OwnerType && OwnerId == Other.OwnerId;
	}
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_ItemStack
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Item", meta = (ClampMin = "0"))
	int32 Quantity = 0;

	bool IsValid() const { return !ItemId.IsNone() && Quantity > 0; }
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_AssignedPalSlot
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Pal")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Pal")
	FName PalId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Pal")
	FName CurrentWorkTargetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Pal")
	FName AssignedState = TEXT("Idle");

	UPROPERTY()
	TObjectPtr<AActor> SpawnedPalActor;
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_BaseWorkState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work")
	FName WorkId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work")
	FName PalId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work")
	FName WorkTargetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work")
	FGameplayTag WorkTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work", meta = (ClampMin = "0.0"))
	float Progress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work", meta = (ClampMin = "0.0"))
	float RequiredProgress = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work", meta = (ClampMin = "0.0"))
	float WorkRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work")
	double LastSimulatedTime = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work")
	bool bCompleted = false;
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_WorkTargetEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work")
	FName WorkTargetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work")
	FGameplayTag RequiredWorkTag;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_WorkRecipe
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work")
	FName RecipeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work")
	TArray<FPW_ItemStack> Ingredients;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Base|Work")
	FPW_ItemStack ResultItem;
};
