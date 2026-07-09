#include "Base/PW_BuildingPieceActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"

namespace
{
FTransform MakeBuildingPlacementTransformFromSnap(const FPW_BuildingSnapPoint& SnapPoint, const FTransform& ParentTransform)
{
	const FTransform ParentSocketTransform = SnapPoint.LocalTransform * ParentTransform;
	return SnapPoint.ChildLocalTransform.Inverse() * ParentSocketTransform;
}
}

APW_BuildingPieceActor::APW_BuildingPieceActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);
	MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
	MeshComponent->SetCanEverAffectNavigation(true);
	MeshComponent->SetVisibility(true, true);
}

void APW_BuildingPieceActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshCodeDefinedSnapPoints();
	PieceType = DefaultPieceType;
	SetMeshVisibleForGameplay();
	ApplyMaterialOverride();
}

void APW_BuildingPieceActor::BeginPlay()
{
	Super::BeginPlay();

	RefreshCodeDefinedSnapPoints();
}

void APW_BuildingPieceActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APW_BuildingPieceActor, PieceType);
	DOREPLIFETIME(APW_BuildingPieceActor, MaterialType);
	DOREPLIFETIME(APW_BuildingPieceActor, CurrentDurability);
	DOREPLIFETIME(APW_BuildingPieceActor, MaxDurability);
	DOREPLIFETIME(APW_BuildingPieceActor, ParentPiece);
	DOREPLIFETIME(APW_BuildingPieceActor, ParentSnapId);
	DOREPLIFETIME(APW_BuildingPieceActor, ChildPieces);
	DOREPLIFETIME(APW_BuildingPieceActor, RuntimeMaterialOverride);
}

void APW_BuildingPieceActor::Destroyed()
{
	if (HasAuthority() && !bIsDestroyingPiece)
	{
		DestroyChildren();
	}

	if (HasAuthority() && ParentPiece)
	{
		ParentPiece->RemoveChildPiece(this);
	}

	Super::Destroyed();
}

void APW_BuildingPieceActor::InitializeBuildingPiece(
	EPW_BuildingPieceType InPieceType,
	EPW_BuildingMaterialType InMaterialType,
	float InMaxDurability,
	UMaterialInterface* InMaterialOverride,
	APW_BuildingPieceActor* InParentPiece,
	FName InParentSnapId)
{
	if (!HasAuthority())
	{
		return;
	}

	PieceType = InPieceType;
	MaterialType = InMaterialType;
	MaxDurability = FMath::Max(1.0f, InMaxDurability);
	CurrentDurability = MaxDurability;
	RuntimeMaterialOverride = InMaterialOverride;
	if (RuntimeMaterialOverride == nullptr)
	{
		RuntimeMaterialOverride = ResolveMaterialOverride();
	}
	ParentPiece = InParentPiece;
	ParentSnapId = InParentSnapId;

	if (ParentPiece)
	{
		ParentPiece->AddChildPiece(this);
	}

	SetMeshVisibleForGameplay();
	ApplyMaterialOverride();
	UE_LOG(LogTemp, Log, TEXT("PW_BuildingPieceActor initialized: %s Piece=%d Material=%d Durability=%.0f Mesh=%s Material=%s"),
		*GetNameSafe(this),
		static_cast<int32>(PieceType),
		static_cast<int32>(MaterialType),
		MaxDurability,
		*GetNameSafe(MeshComponent ? MeshComponent->GetStaticMesh() : nullptr),
		*GetNameSafe(RuntimeMaterialOverride.Get()));
	BP_OnBuildingPieceStateChanged();
	BP_OnDurabilityChanged();
}

void APW_BuildingPieceActor::ApplyBuildingDamage(float DamageAmount)
{
	if (!HasAuthority() || DamageAmount <= 0.0f)
	{
		return;
	}

	CurrentDurability = FMath::Clamp(CurrentDurability - DamageAmount, 0.0f, MaxDurability);
	BP_OnDurabilityChanged();

	if (CurrentDurability <= 0.0f)
	{
		DestroyPiece();
	}
}

void APW_BuildingPieceActor::DestroyPiece()
{
	if (!HasAuthority() || bIsDestroyingPiece)
	{
		return;
	}

	bIsDestroyingPiece = true;
	DestroyChildren();
	Destroy();
}

bool APW_BuildingPieceActor::FindSnapPoint(FName SnapId, FPW_BuildingSnapPoint& OutSnapPoint) const
{
	for (const FPW_BuildingSnapPoint& SnapPoint : SnapPoints)
	{
		if (SnapPoint.SnapId == SnapId)
		{
			OutSnapPoint = SnapPoint;
			return true;
		}
	}

	return false;
}

bool APW_BuildingPieceActor::FindNearestCompatibleSnapPoint(
	EPW_BuildingPieceType DesiredPieceType,
	const FVector& QueryLocation,
	FPW_BuildingSnapPoint& OutSnapPoint,
	FTransform& OutSnapTransform) const
{
	bool bFoundSnapPoint = false;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (const FPW_BuildingSnapPoint& SnapPoint : SnapPoints)
	{
		if (SnapPoint.AcceptsPieceType != DesiredPieceType || !IsSnapPointAvailable(SnapPoint.SnapId))
		{
			continue;
		}

		const FTransform ParentSocketTransform = SnapPoint.LocalTransform * GetActorTransform();
		const FVector SnapQueryLocation = ParentSocketTransform.GetLocation();

		const float DistanceSquared = FVector::DistSquared(SnapQueryLocation, QueryLocation);
		if (DistanceSquared <= FMath::Square(SnapPoint.SnapRadius) && DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			OutSnapPoint = SnapPoint;
			OutSnapTransform = MakeBuildingPlacementTransformFromSnap(SnapPoint, GetActorTransform());
			bFoundSnapPoint = true;
		}
	}

	return bFoundSnapPoint;
}

bool APW_BuildingPieceActor::IsSnapPointAvailable(FName SnapId) const
{
	if (SnapId.IsNone())
	{
		return true;
	}

	for (const APW_BuildingPieceActor* Child : ChildPieces)
	{
		if (IsValid(Child) && Child->GetParentSnapId() == SnapId)
		{
			return false;
		}
	}

	return true;
}

void APW_BuildingPieceActor::AddChildPiece(APW_BuildingPieceActor* ChildPiece)
{
	if (!HasAuthority() || ChildPiece == nullptr || ChildPiece == this)
	{
		return;
	}

	ChildPieces.AddUnique(ChildPiece);
}

void APW_BuildingPieceActor::RemoveChildPiece(APW_BuildingPieceActor* ChildPiece)
{
	if (!HasAuthority() || ChildPiece == nullptr)
	{
		return;
	}

	ChildPieces.Remove(ChildPiece);
}

void APW_BuildingPieceActor::ApplyPreviewMaterial(UMaterialInterface* PreviewMaterial)
{
	if (!MeshComponent || PreviewMaterial == nullptr)
	{
		return;
	}

	const int32 MaterialCount = FMath::Max(1, MeshComponent->GetNumMaterials());
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		MeshComponent->SetMaterial(MaterialIndex, PreviewMaterial);
	}
}

void APW_BuildingPieceActor::ClearPreviewMaterial()
{
	ApplyMaterialOverride();
}

void APW_BuildingPieceActor::BuildCodeDefinedSnapPoints(TArray<FPW_BuildingSnapPoint>& OutSnapPoints) const
{
}

FPW_BuildingSnapPoint APW_BuildingPieceActor::MakeSnapPoint(
	FName SnapId,
	EPW_BuildingPieceType AcceptsPieceType,
	const FVector& LocalLocation,
	float LocalYaw,
	float SnapRadius,
	const FVector& ChildLocalLocation,
	float ChildLocalYaw)
{
	FPW_BuildingSnapPoint SnapPoint;
	SnapPoint.SnapId = SnapId;
	SnapPoint.AcceptsPieceType = AcceptsPieceType;
	SnapPoint.LocalTransform = FTransform(FRotator(0.0f, LocalYaw, 0.0f), LocalLocation);
	SnapPoint.ChildLocalTransform = FTransform(FRotator(0.0f, ChildLocalYaw, 0.0f), ChildLocalLocation);
	SnapPoint.SnapRadius = SnapRadius;
	return SnapPoint;
}

void APW_BuildingPieceActor::OnRep_PieceState()
{
	ApplyMaterialOverride();
	BP_OnBuildingPieceStateChanged();
}

void APW_BuildingPieceActor::OnRep_Durability()
{
	BP_OnDurabilityChanged();
}

void APW_BuildingPieceActor::ApplyMaterialOverride()
{
	UMaterialInterface* MaterialOverride = RuntimeMaterialOverride ? RuntimeMaterialOverride.Get() : ResolveMaterialOverride();
	if (MeshComponent == nullptr || MaterialOverride == nullptr)
	{
		return;
	}

	const int32 MaterialCount = FMath::Max(1, MeshComponent->GetNumMaterials());
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		MeshComponent->SetMaterial(MaterialIndex, MaterialOverride);
	}
}

UMaterialInterface* APW_BuildingPieceActor::ResolveMaterialOverride() const
{
	for (const FPW_BuildingMaterialProfile& MaterialProfile : MaterialProfiles)
	{
		if (MaterialProfile.MaterialType == MaterialType)
		{
			return MaterialProfile.MaterialOverride.Get();
		}
	}

	return nullptr;
}

void APW_BuildingPieceActor::RefreshCodeDefinedSnapPoints()
{
	if (!bUseCodeDefinedSnapPoints)
	{
		return;
	}

	TArray<FPW_BuildingSnapPoint> GeneratedSnapPoints;
	BuildCodeDefinedSnapPoints(GeneratedSnapPoints);
	SnapPoints = MoveTemp(GeneratedSnapPoints);
}

void APW_BuildingPieceActor::SetMeshVisibleForGameplay()
{
	if (MeshComponent == nullptr)
	{
		return;
	}

	MeshComponent->SetHiddenInGame(false, true);
	MeshComponent->SetVisibility(true, true);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void APW_BuildingPieceActor::DestroyChildren()
{
	TArray<TObjectPtr<APW_BuildingPieceActor>> ChildrenCopy = ChildPieces;
	ChildPieces.Reset();

	for (APW_BuildingPieceActor* Child : ChildrenCopy)
	{
		if (IsValid(Child))
		{
			Child->DestroyPiece();
		}
	}
}
