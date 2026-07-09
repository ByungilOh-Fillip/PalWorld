#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BuildingPieceActor.h"
#include "PW_BuildingWallActor.generated.h"

UCLASS(Blueprintable)
class PALWORLD_API APW_BuildingWallActor : public APW_BuildingPieceActor
{
	GENERATED_BODY()

public:
	APW_BuildingWallActor();

protected:
	virtual void BuildCodeDefinedSnapPoints(TArray<FPW_BuildingSnapPoint>& OutSnapPoints) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Wall")
	float RoofSocketHeight = 340.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Wall")
	float RoofSnapRadius = 220.0f;
};
