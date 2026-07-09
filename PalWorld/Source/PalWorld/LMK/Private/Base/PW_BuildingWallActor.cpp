#include "Base/PW_BuildingWallActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

APW_BuildingWallActor::APW_BuildingWallActor()
{
	DefaultPieceType = EPW_BuildingPieceType::Wall;
	PieceType = EPW_BuildingPieceType::Wall;
	bUseCodeDefinedSnapPoints = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WallMesh(
		TEXT("/Game/_Private/LMK/Asset/EasyBuildingSystem/Meshes/Structures/Stylized/Wood/SM_Stylized_Wood_Wall.SM_Stylized_Wood_Wall"));
	if (WallMesh.Succeeded() && MeshComponent)
	{
		MeshComponent->SetStaticMesh(WallMesh.Object);
	}
}

void APW_BuildingWallActor::BuildCodeDefinedSnapPoints(TArray<FPW_BuildingSnapPoint>& OutSnapPoints) const
{
	OutSnapPoints.Reset();

	OutSnapPoints.Add(MakeSnapPoint(
		FName(TEXT("Roof_Top")),
		EPW_BuildingPieceType::Roof,
		FVector(0.0f, 0.0f, RoofSocketHeight),
		0.0f,
		RoofSnapRadius));
}
