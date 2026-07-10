#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BuildingPieceActor.h"
#include "PW_BuildingFoundationActor.generated.h"

UCLASS(Blueprintable)
class PALWORLD_API APW_BuildingFoundationActor : public APW_BuildingPieceActor
{
	GENERATED_BODY()

public:
	APW_BuildingFoundationActor();

	bool ResolveSnapPlacement(
		EPW_BuildingPieceType DesiredPieceType,
		const FVector& WorldQueryLocation,
		float YawOffset,
		FTransform& OutPlacementTransform,
		FName& OutSnapId) const;

	bool ResolveSnapPlacementById(
		EPW_BuildingPieceType DesiredPieceType,
		FName SnapId,
		float YawOffset,
		FTransform& OutPlacementTransform) const;

protected:
	virtual void BuildCodeDefinedSnapPoints(TArray<FPW_BuildingSnapPoint>& OutSnapPoints) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Foundation")
	float FoundationSocketDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Foundation")
	float WallSocketForwardDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Foundation")
	float WallSocketHeight = 235.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Foundation")
	float FoundationSnapRadius = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Foundation")
	float WallSnapRadius = 220.0f;

private:
	FName SelectSnapId(EPW_BuildingPieceType DesiredPieceType, const FVector& WorldQueryLocation) const;
	FVector GetParentSocketLocalLocation(FName SnapId) const;
	FVector GetChildSocketLocalLocation(FName SnapId) const;
	float GetSnapYaw(FName SnapId, float YawOffset) const;
};
