#pragma once

#include "CoreMinimal.h"
#include "PW_MapTypes.generated.h"

UENUM(BlueprintType)
enum class EPW_MapMarkerType : uint8
{
	Player,
	BaseCamp,
	TeleportPoint
};

UENUM(BlueprintType)
enum class EPW_MapExplorationState : uint8
{
	Unvisited,
	Visited
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_MapMarker
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Map")
	EPW_MapMarkerType MarkerType = EPW_MapMarkerType::TeleportPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Map")
	FName MarkerId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Map")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Map")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Map")
	FVector2D MapUV = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Map")
	bool bDiscovered = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Map")
	bool bCanTeleport = false;
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_MapExplorationSaveData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Map")
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Map")
	FName MapId = TEXT("Default");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Map")
	int32 GridWidth = 128;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Map")
	int32 GridHeight = 128;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Map")
	TArray<int32> VisitedCellIndices;

	bool IsCompatible(FName CurrentMapId, int32 CurrentGridWidth, int32 CurrentGridHeight) const
	{
		return MapId == CurrentMapId && GridWidth == CurrentGridWidth && GridHeight == CurrentGridHeight;
	}
};

