#include "Base/PW_BuildingFoundationActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

APW_BuildingFoundationActor::APW_BuildingFoundationActor()
{
	DefaultPieceType = EPW_BuildingPieceType::Foundation;
	PieceType = EPW_BuildingPieceType::Foundation;
	bUseCodeDefinedSnapPoints = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FoundationMesh(
		TEXT("/Game/_Private/LMK/Asset/EasyBuildingSystem/Meshes/Structures/Stylized/Wood/SM_Stylized_Wood_Foundation.SM_Stylized_Wood_Foundation"));
	if (FoundationMesh.Succeeded() && MeshComponent)
	{
		MeshComponent->SetStaticMesh(FoundationMesh.Object);
	}
}

bool APW_BuildingFoundationActor::ResolveSnapPlacement(
	EPW_BuildingPieceType DesiredPieceType,
	const FVector& WorldQueryLocation,
	float YawOffset,
	FTransform& OutPlacementTransform,
	FName& OutSnapId) const
{
	OutSnapId = SelectSnapId(DesiredPieceType, WorldQueryLocation);
	if (OutSnapId.IsNone())
	{
		return false;
	}

	return ResolveSnapPlacementById(DesiredPieceType, OutSnapId, YawOffset, OutPlacementTransform);
}

bool APW_BuildingFoundationActor::ResolveSnapPlacementById(
	EPW_BuildingPieceType DesiredPieceType,
	FName SnapId,
	float YawOffset,
	FTransform& OutPlacementTransform) const
{
	if (DesiredPieceType != EPW_BuildingPieceType::Foundation && DesiredPieceType != EPW_BuildingPieceType::Wall)
	{
		return false;
	}

	const FVector ParentSocketLocalLocation = GetParentSocketLocalLocation(SnapId);
	if (ParentSocketLocalLocation.IsNearlyZero() && !SnapId.IsEqual(FName(TEXT("Foundation_Center"))))
	{
		return false;
	}

	const float PlacementYaw = GetSnapYaw(SnapId, YawOffset);
	const FRotator PlacementRotation(0.0f, PlacementYaw, 0.0f);
	const FVector ParentSocketWorldLocation = GetActorTransform().TransformPosition(ParentSocketLocalLocation);
	const FVector ChildSocketOffset = PlacementRotation.RotateVector(GetChildSocketLocalLocation(SnapId));

	OutPlacementTransform = FTransform(PlacementRotation, ParentSocketWorldLocation - ChildSocketOffset);
	return true;
}

void APW_BuildingFoundationActor::BuildCodeDefinedSnapPoints(TArray<FPW_BuildingSnapPoint>& OutSnapPoints) const
{
	OutSnapPoints.Reset();

	const float FoundationDistance = FMath::Max(1.0f, FoundationSocketDistance);
	const float WallForwardDistance = FMath::Max(1.0f, WallSocketForwardDistance);

	OutSnapPoints.Add(MakeSnapPoint(
		FName(TEXT("Foundation_North")),
		EPW_BuildingPieceType::Foundation,
		FVector(0.0f, FoundationDistance, 0.0f),
		0.0f,
		FoundationSnapRadius,
		FVector(0.0f, -FoundationDistance, 0.0f)));
	OutSnapPoints.Add(MakeSnapPoint(
		FName(TEXT("Foundation_East")),
		EPW_BuildingPieceType::Foundation,
		FVector(FoundationDistance, 0.0f, 0.0f),
		0.0f,
		FoundationSnapRadius,
		FVector(-FoundationDistance, 0.0f, 0.0f)));
	OutSnapPoints.Add(MakeSnapPoint(
		FName(TEXT("Foundation_South")),
		EPW_BuildingPieceType::Foundation,
		FVector(0.0f, -FoundationDistance, 0.0f),
		0.0f,
		FoundationSnapRadius,
		FVector(0.0f, FoundationDistance, 0.0f)));
	OutSnapPoints.Add(MakeSnapPoint(
		FName(TEXT("Foundation_West")),
		EPW_BuildingPieceType::Foundation,
		FVector(-FoundationDistance, 0.0f, 0.0f),
		0.0f,
		FoundationSnapRadius,
		FVector(FoundationDistance, 0.0f, 0.0f)));

	OutSnapPoints.Add(MakeSnapPoint(
		FName(TEXT("Wall_North")),
		EPW_BuildingPieceType::Wall,
		FVector(0.0f, WallForwardDistance, WallSocketHeight),
		0.0f,
		WallSnapRadius));
	OutSnapPoints.Add(MakeSnapPoint(
		FName(TEXT("Wall_East")),
		EPW_BuildingPieceType::Wall,
		FVector(WallForwardDistance, 0.0f, WallSocketHeight),
		90.0f,
		WallSnapRadius));
	OutSnapPoints.Add(MakeSnapPoint(
		FName(TEXT("Wall_South")),
		EPW_BuildingPieceType::Wall,
		FVector(0.0f, -WallForwardDistance, WallSocketHeight),
		180.0f,
		WallSnapRadius));
	OutSnapPoints.Add(MakeSnapPoint(
		FName(TEXT("Wall_West")),
		EPW_BuildingPieceType::Wall,
		FVector(-WallForwardDistance, 0.0f, WallSocketHeight),
		-90.0f,
		WallSnapRadius));
}

FName APW_BuildingFoundationActor::SelectSnapId(EPW_BuildingPieceType DesiredPieceType, const FVector& WorldQueryLocation) const
{
	if (DesiredPieceType != EPW_BuildingPieceType::Foundation && DesiredPieceType != EPW_BuildingPieceType::Wall)
	{
		return NAME_None;
	}

	const FVector LocalQueryLocation = GetActorTransform().InverseTransformPosition(WorldQueryLocation);
	const bool bUseXAxis = FMath::Abs(LocalQueryLocation.X) > FMath::Abs(LocalQueryLocation.Y);

	if (DesiredPieceType == EPW_BuildingPieceType::Foundation)
	{
		if (bUseXAxis)
		{
			return LocalQueryLocation.X >= 0.0f ? FName(TEXT("Foundation_East")) : FName(TEXT("Foundation_West"));
		}

		return LocalQueryLocation.Y >= 0.0f ? FName(TEXT("Foundation_North")) : FName(TEXT("Foundation_South"));
	}

	if (bUseXAxis)
	{
		return LocalQueryLocation.X >= 0.0f ? FName(TEXT("Wall_East")) : FName(TEXT("Wall_West"));
	}

	return LocalQueryLocation.Y >= 0.0f ? FName(TEXT("Wall_North")) : FName(TEXT("Wall_South"));
}

FVector APW_BuildingFoundationActor::GetParentSocketLocalLocation(FName SnapId) const
{
	const float FoundationDistance = FMath::Max(1.0f, FoundationSocketDistance);
	const float WallDistance = FMath::Max(1.0f, WallSocketForwardDistance);

	if (SnapId.IsEqual(FName(TEXT("Foundation_North"))))
	{
		return FVector(0.0f, FoundationDistance, 0.0f);
	}
	if (SnapId.IsEqual(FName(TEXT("Foundation_East"))))
	{
		return FVector(FoundationDistance, 0.0f, 0.0f);
	}
	if (SnapId.IsEqual(FName(TEXT("Foundation_South"))))
	{
		return FVector(0.0f, -FoundationDistance, 0.0f);
	}
	if (SnapId.IsEqual(FName(TEXT("Foundation_West"))))
	{
		return FVector(-FoundationDistance, 0.0f, 0.0f);
	}
	if (SnapId.IsEqual(FName(TEXT("Wall_North"))))
	{
		return FVector(0.0f, WallDistance, WallSocketHeight);
	}
	if (SnapId.IsEqual(FName(TEXT("Wall_East"))))
	{
		return FVector(WallDistance, 0.0f, WallSocketHeight);
	}
	if (SnapId.IsEqual(FName(TEXT("Wall_South"))))
	{
		return FVector(0.0f, -WallDistance, WallSocketHeight);
	}
	if (SnapId.IsEqual(FName(TEXT("Wall_West"))))
	{
		return FVector(-WallDistance, 0.0f, WallSocketHeight);
	}

	return FVector::ZeroVector;
}

FVector APW_BuildingFoundationActor::GetChildSocketLocalLocation(FName SnapId) const
{
	const float FoundationDistance = FMath::Max(1.0f, FoundationSocketDistance);

	if (SnapId.IsEqual(FName(TEXT("Foundation_North"))))
	{
		return FVector(0.0f, -FoundationDistance, 0.0f);
	}
	if (SnapId.IsEqual(FName(TEXT("Foundation_East"))))
	{
		return FVector(-FoundationDistance, 0.0f, 0.0f);
	}
	if (SnapId.IsEqual(FName(TEXT("Foundation_South"))))
	{
		return FVector(0.0f, FoundationDistance, 0.0f);
	}
	if (SnapId.IsEqual(FName(TEXT("Foundation_West"))))
	{
		return FVector(FoundationDistance, 0.0f, 0.0f);
	}

	return FVector::ZeroVector;
}

float APW_BuildingFoundationActor::GetSnapYaw(FName SnapId, float YawOffset) const
{
	if (SnapId.ToString().StartsWith(TEXT("Foundation_")))
	{
		return GetActorRotation().Yaw;
	}
	if (SnapId.IsEqual(FName(TEXT("Wall_North"))))
	{
		return FMath::GridSnap(FMath::UnwindDegrees(GetActorRotation().Yaw + YawOffset), 90.0f);
	}
	if (SnapId.IsEqual(FName(TEXT("Wall_East"))))
	{
		return FMath::GridSnap(FMath::UnwindDegrees(GetActorRotation().Yaw + 90.0f + YawOffset), 90.0f);
	}
	if (SnapId.IsEqual(FName(TEXT("Wall_South"))))
	{
		return FMath::GridSnap(FMath::UnwindDegrees(GetActorRotation().Yaw + 180.0f + YawOffset), 90.0f);
	}
	if (SnapId.IsEqual(FName(TEXT("Wall_West"))))
	{
		return FMath::GridSnap(FMath::UnwindDegrees(GetActorRotation().Yaw - 90.0f + YawOffset), 90.0f);
	}

	return GetActorRotation().Yaw;
}
