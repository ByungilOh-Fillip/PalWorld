#include "Base/PW_BuildingRoofActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

APW_BuildingRoofActor::APW_BuildingRoofActor()
{
	DefaultPieceType = EPW_BuildingPieceType::Roof;
	PieceType = EPW_BuildingPieceType::Roof;
	bUseCodeDefinedSnapPoints = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> RoofMesh(
		TEXT("/Game/_Private/LMK/Asset/EasyBuildingSystem/Meshes/Structures/Stylized/Wood/SM_Stylized_Wood_Roof.SM_Stylized_Wood_Roof"));
	if (RoofMesh.Succeeded() && MeshComponent)
	{
		MeshComponent->SetStaticMesh(RoofMesh.Object);
	}
}
