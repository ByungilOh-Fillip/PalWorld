#pragma once

#include "CoreMinimal.h"
#include "PW_BuildingTypes.generated.h"

class APW_BuildingPieceActor;
class UMaterialInterface;
class UTexture2D;

UENUM(BlueprintType)
enum class EPW_BuildingPieceType : uint8
{
	Foundation,
	Wall,
	Roof
};

UENUM(BlueprintType)
enum class EPW_BuildingMaterialType : uint8
{
	Wood,
	Stone,
	Iron
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_BuildingMaterialProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	EPW_BuildingMaterialType MaterialType = EPW_BuildingMaterialType::Wood;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	TObjectPtr<UMaterialInterface> MaterialOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building", meta = (ClampMin = "1.0"))
	float MaxDurability = 500.0f;
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_BuildingSnapPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	FName SnapId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	EPW_BuildingPieceType AcceptsPieceType = EPW_BuildingPieceType::Wall;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	FTransform LocalTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	FTransform ChildLocalTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building", meta = (ClampMin = "0.0"))
	float SnapRadius = 120.0f;
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_BuildingPieceDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	EPW_BuildingPieceType PieceType = EPW_BuildingPieceType::Foundation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	TSubclassOf<APW_BuildingPieceActor> PieceClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building", meta = (ClampMin = "0.0"))
	float RotationStepDegrees = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	TObjectPtr<UMaterialInterface> PreviewValidMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	TObjectPtr<UMaterialInterface> PreviewInvalidMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	FVector PlacementOverlapExtent = FVector(120.0f, 120.0f, 120.0f);
};
