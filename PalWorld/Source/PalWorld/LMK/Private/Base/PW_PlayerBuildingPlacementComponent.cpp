#include "Base/PW_PlayerBuildingPlacementComponent.h"

#include "Base/PW_BuildingFoundationActor.h"
#include "Base/PW_BuildingPieceActor.h"
#include "Base/PW_BuildingRoofActor.h"
#include "Base/PW_BuildingWallActor.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Materials/MaterialInterface.h"

UPW_PlayerBuildingPlacementComponent::UPW_PlayerBuildingPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void UPW_PlayerBuildingPlacementComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureDefaultMaterialProfiles();
	EnsureDefaultPieceDefinitions();
	SetComponentTickEnabled(false);
}

void UPW_PlayerBuildingPlacementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyPreviewActor();
	if (IsValid(DismantleTargetPiece))
	{
		DismantleTargetPiece->ClearPreviewMaterial();
		DismantleTargetPiece = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void UPW_PlayerBuildingPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDismantleModeActive)
	{
		UpdateDismantlePreview();
	}
	else if (bPlacementModeActive)
	{
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		APlayerController* PlayerController = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
		if (PlayerController)
		{
			const float MouseWheelAxis = PlayerController->GetInputAnalogKeyState(EKeys::MouseWheelAxis);
			if (!FMath::IsNearlyZero(MouseWheelAxis) && FMath::IsNearlyZero(LastPolledMouseWheelAxis))
			{
				RotatePreviewByWheel(MouseWheelAxis);
			}
			LastPolledMouseWheelAxis = MouseWheelAxis;
		}

		UpdatePlacementPreview();
	}
}

void UPW_PlayerBuildingPlacementComponent::EnterPlacementMode(EPW_BuildingPieceType PieceType, EPW_BuildingMaterialType MaterialType)
{
	SelectedPieceType = PieceType;
	SelectedMaterialType = MaterialType;
	CurrentYawOffset = 0.0f;
	LastPolledMouseWheelAxis = 0.0f;
	CurrentFoundationHeightOffset = 0.0f;
	CurrentParentPiece = nullptr;
	CurrentParentSnapId = NAME_None;
	SetPlacementModeActive(true);
}

void UPW_PlayerBuildingPlacementComponent::CancelPlacement()
{
	SetPlacementModeActive(false);
}

void UPW_PlayerBuildingPlacementComponent::ConfirmDismantle()
{
	if (!bDismantleModeActive || !IsValid(DismantleTargetPiece))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("PW_PlayerBuildingPlacementComponent request dismantle piece: %s"), *GetNameSafe(DismantleTargetPiece));
	ServerRequestDismantlePiece(DismantleTargetPiece);
	
	if (IsValid(DismantleTargetPiece))
	{
		DismantleTargetPiece->ClearPreviewMaterial();
		DismantleTargetPiece = nullptr;
	}
}

void UPW_PlayerBuildingPlacementComponent::ConfirmPlacement()
{
	if (!bPlacementModeActive)
	{
		return;
	}

	UpdatePlacementPreview();
	if (!bCanPlaceAtCurrentPreview)
	{
		UE_LOG(LogTemp, Warning, TEXT("PW_PlayerBuildingPlacementComponent confirm rejected: current preview invalid. Piece=%d Material=%d"),
			static_cast<int32>(SelectedPieceType),
			static_cast<int32>(SelectedMaterialType));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("PW_PlayerBuildingPlacementComponent confirm placement request. Piece=%d Material=%d Location=%s Parent=%s Snap=%s"),
		static_cast<int32>(SelectedPieceType),
		static_cast<int32>(SelectedMaterialType),
		*CurrentPlacementTransform.GetLocation().ToCompactString(),
		*GetNameSafe(CurrentParentPiece),
		*CurrentParentSnapId.ToString());
	ServerRequestPlacePiece(SelectedPieceType, SelectedMaterialType, CurrentPlacementTransform, CurrentParentPiece, CurrentParentSnapId);
}

void UPW_PlayerBuildingPlacementComponent::RotatePreviewByWheel(float WheelDelta)
{
	if (!bPlacementModeActive || FMath::IsNearlyZero(WheelDelta))
	{
		return;
	}

	const FPW_BuildingPieceDefinition* PieceDefinition = FindPieceDefinition(SelectedPieceType);
	const float RotationStep = PieceDefinition ? PieceDefinition->RotationStepDegrees : 15.0f;
	const float AppliedRotationStep = RotationStep > 0.0f ? RotationStep : 5.0f;
	CurrentYawOffset = FMath::UnwindDegrees(CurrentYawOffset + FMath::Sign(WheelDelta) * AppliedRotationStep);
	UpdatePlacementPreview();
}

void UPW_PlayerBuildingPlacementComponent::ApplyFoundationHeightMouseDelta(float MouseYDelta)
{
	// Height adjustment is intentionally disabled for v1; foundation placement should follow trace/snap only.
}

bool UPW_PlayerBuildingPlacementComponent::TryConsumeLookInput(const FVector2D& LookVector)
{
	return false;
}

void UPW_PlayerBuildingPlacementComponent::ServerRequestPlacePiece_Implementation(
	EPW_BuildingPieceType PieceType,
	EPW_BuildingMaterialType MaterialType,
	FTransform PlacementTransform,
	APW_BuildingPieceActor* ParentPiece,
	FName ParentSnapId)
{
	const FPW_BuildingPieceDefinition* PieceDefinition = FindPieceDefinition(PieceType);
	const FPW_BuildingMaterialProfile* MaterialProfile = FindMaterialProfile(MaterialType);
	if (PieceDefinition == nullptr || MaterialProfile == nullptr || PieceDefinition->PieceClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PW_PlayerBuildingPlacementComponent server rejected: missing definition/profile/class. Piece=%d Material=%d"),
			static_cast<int32>(PieceType),
			static_cast<int32>(MaterialType));
		return;
	}

	if (!ValidateServerPlacement(*PieceDefinition, ParentPiece, ParentSnapId, PlacementTransform))
	{
		UE_LOG(LogTemp, Warning, TEXT("PW_PlayerBuildingPlacementComponent server rejected: validation failed. Piece=%d Material=%d Location=%s Parent=%s Snap=%s"),
			static_cast<int32>(PieceType),
			static_cast<int32>(MaterialType),
			*PlacementTransform.GetLocation().ToCompactString(),
			*GetNameSafe(ParentPiece),
			*ParentSnapId.ToString());
		return;
	}

	SpawnBuildingPieceAuthority(*PieceDefinition, *MaterialProfile, PlacementTransform, ParentPiece, ParentSnapId);
}

void UPW_PlayerBuildingPlacementComponent::ServerRequestDismantlePiece_Implementation(APW_BuildingPieceActor* TargetPiece)
{
	if (IsValid(TargetPiece))
	{
		TargetPiece->DestroyPiece();
	}
}

const FPW_BuildingPieceDefinition* UPW_PlayerBuildingPlacementComponent::FindPieceDefinition(EPW_BuildingPieceType PieceType) const
{
	for (const FPW_BuildingPieceDefinition& PieceDefinition : PieceDefinitions)
	{
		if (PieceDefinition.PieceType == PieceType)
		{
			return &PieceDefinition;
		}
	}

	return nullptr;
}

const FPW_BuildingMaterialProfile* UPW_PlayerBuildingPlacementComponent::FindMaterialProfile(EPW_BuildingMaterialType MaterialType) const
{
	for (const FPW_BuildingMaterialProfile& MaterialProfile : MaterialProfiles)
	{
		if (MaterialProfile.MaterialType == MaterialType)
		{
			return &MaterialProfile;
		}
	}

	return nullptr;
}

void UPW_PlayerBuildingPlacementComponent::EnsureDefaultMaterialProfiles()
{
	UMaterialInterface* WoodMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/_Private/LMK/Asset/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Wood.MI_Stylized_Structures_Wood"));
	UMaterialInterface* StoneMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/_Private/LMK/Asset/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Stone.MI_Stylized_Structures_Stone"));
	UMaterialInterface* IronMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/_Private/LMK/Asset/EasyBuildingSystem/Materials/Instances/Stylized/MI_Stylized_Structures_Metal.MI_Stylized_Structures_Metal"));

	auto UpsertMaterialProfile = [this](EPW_BuildingMaterialType MaterialType, const FText& DisplayName, UMaterialInterface* MaterialOverride, float Durability)
	{
		for (FPW_BuildingMaterialProfile& MaterialProfile : MaterialProfiles)
		{
			if (MaterialProfile.MaterialType == MaterialType)
			{
				MaterialProfile.DisplayName = DisplayName;
				MaterialProfile.MaxDurability = Durability;
				if (MaterialProfile.MaterialOverride == nullptr)
				{
					MaterialProfile.MaterialOverride = MaterialOverride;
				}
				return;
			}
		}

		FPW_BuildingMaterialProfile MaterialProfile;
		MaterialProfile.MaterialType = MaterialType;
		MaterialProfile.DisplayName = DisplayName;
		MaterialProfile.MaterialOverride = MaterialOverride;
		MaterialProfile.MaxDurability = Durability;
		MaterialProfiles.Add(MaterialProfile);
	};

	UpsertMaterialProfile(EPW_BuildingMaterialType::Wood, NSLOCTEXT("PWBuilding", "WoodMaterial", "Wood"), WoodMaterial, 500.0f);
	UpsertMaterialProfile(EPW_BuildingMaterialType::Stone, NSLOCTEXT("PWBuilding", "StoneMaterial", "Stone"), StoneMaterial, 1000.0f);
	UpsertMaterialProfile(EPW_BuildingMaterialType::Iron, NSLOCTEXT("PWBuilding", "IronMaterial", "Iron"), IronMaterial, 2000.0f);
}

void UPW_PlayerBuildingPlacementComponent::EnsureDefaultPieceDefinitions()
{
	TSubclassOf<APW_BuildingPieceActor> FoundationClass = APW_BuildingFoundationActor::StaticClass();
	TSubclassOf<APW_BuildingPieceActor> WallClass = APW_BuildingWallActor::StaticClass();
	TSubclassOf<APW_BuildingPieceActor> RoofClass = APW_BuildingRoofActor::StaticClass();
	UMaterialInterface* PreviewValidMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/_Private/LMK/Asset/EasyBuildingSystem/Materials/Instances/Dummy/MI_Can_Build.MI_Can_Build"));
	UMaterialInterface* PreviewInvalidMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/_Private/LMK/Asset/EasyBuildingSystem/Materials/Instances/Dummy/MI_CanNot_Build.MI_CanNot_Build"));

	auto UpsertPieceDefinition = [this, PreviewValidMaterial, PreviewInvalidMaterial](
		EPW_BuildingPieceType PieceType,
		const FText& DisplayName,
		TSubclassOf<APW_BuildingPieceActor> PieceClass,
		float RotationStepDegrees,
		const FVector& OverlapExtent)
	{
		for (FPW_BuildingPieceDefinition& PieceDefinition : PieceDefinitions)
		{
			if (PieceDefinition.PieceType == PieceType)
			{
				PieceDefinition.DisplayName = DisplayName;
				PieceDefinition.RotationStepDegrees = RotationStepDegrees;
				if (PieceDefinition.PieceClass == nullptr || !PieceDefinition.PieceClass->IsChildOf(PieceClass))
				{
					PieceDefinition.PieceClass = PieceClass;
				}
				if (PieceDefinition.PreviewValidMaterial == nullptr)
				{
					PieceDefinition.PreviewValidMaterial = PreviewValidMaterial;
				}
				if (PieceDefinition.PreviewInvalidMaterial == nullptr)
				{
					PieceDefinition.PreviewInvalidMaterial = PreviewInvalidMaterial;
				}
				if (PieceDefinition.PlacementOverlapExtent.IsNearlyZero())
				{
					PieceDefinition.PlacementOverlapExtent = OverlapExtent;
				}
				return;
			}
		}

		FPW_BuildingPieceDefinition PieceDefinition;
		PieceDefinition.PieceType = PieceType;
		PieceDefinition.DisplayName = DisplayName;
		PieceDefinition.PieceClass = PieceClass;
		PieceDefinition.RotationStepDegrees = RotationStepDegrees;
		PieceDefinition.PreviewValidMaterial = PreviewValidMaterial;
		PieceDefinition.PreviewInvalidMaterial = PreviewInvalidMaterial;
		PieceDefinition.PlacementOverlapExtent = OverlapExtent;
		PieceDefinitions.Add(PieceDefinition);
	};

	UpsertPieceDefinition(EPW_BuildingPieceType::Foundation, NSLOCTEXT("PWBuilding", "FoundationDefinition", "Foundation"), FoundationClass, 5.0f, FVector(220.0f, 220.0f, 55.0f));
	UpsertPieceDefinition(EPW_BuildingPieceType::Wall, NSLOCTEXT("PWBuilding", "WallDefinition", "Wall"), WallClass, 90.0f, FVector(220.0f, 40.0f, 180.0f));
	UpsertPieceDefinition(EPW_BuildingPieceType::Roof, NSLOCTEXT("PWBuilding", "RoofDefinition", "Roof"), RoofClass, 90.0f, FVector(220.0f, 220.0f, 80.0f));
}

void UPW_PlayerBuildingPlacementComponent::SetPlacementModeActive(bool bNewActive)
{
	if (bPlacementModeActive == bNewActive)
	{
		return;
	}

	bPlacementModeActive = bNewActive;
	if (bPlacementModeActive && bDismantleModeActive)
	{
		SetDismantleModeActive(false);
	}
	SetComponentTickEnabled(bPlacementModeActive || bDismantleModeActive);

	if (!bPlacementModeActive)
	{
		bCanPlaceAtCurrentPreview = false;
		CurrentParentPiece = nullptr;
		CurrentParentSnapId = NAME_None;
		DestroyPreviewActor();
		BP_OnPlacementModeChanged(false);
		BP_OnPlacementPreviewChanged(false);
		return;
	}

	UpdatePlacementPreview();
	BP_OnPlacementModeChanged(true);
}

void UPW_PlayerBuildingPlacementComponent::SetDismantleModeActive(bool bNewActive)
{
	if (bDismantleModeActive == bNewActive)
	{
		return;
	}

	bDismantleModeActive = bNewActive;
	if (bDismantleModeActive && bPlacementModeActive)
	{
		SetPlacementModeActive(false);
	}
	SetComponentTickEnabled(bPlacementModeActive || bDismantleModeActive);

	if (!bDismantleModeActive)
	{
		if (IsValid(DismantleTargetPiece))
		{
			DismantleTargetPiece->ClearPreviewMaterial();
			DismantleTargetPiece = nullptr;
		}
		BP_OnDismantleModeChanged(false);
		return;
	}

	UpdateDismantlePreview();
	BP_OnDismantleModeChanged(true);
}

void UPW_PlayerBuildingPlacementComponent::UpdatePlacementPreview()
{
	const FPW_BuildingPieceDefinition* PieceDefinition = FindPieceDefinition(SelectedPieceType);
	if (PieceDefinition == nullptr || PieceDefinition->PieceClass == nullptr)
	{
		bCanPlaceAtCurrentPreview = false;
		DestroyPreviewActor();
		BP_OnPlacementPreviewChanged(false);
		return;
	}

	FTransform PlacementTransform;
	APW_BuildingPieceActor* ParentPiece = nullptr;
	FName ParentSnapId = NAME_None;
	bool bResolvedTransform = false;
	if (SelectedPieceType == EPW_BuildingPieceType::Foundation)
	{
		bResolvedTransform = ResolveSnapPlacementTransform(PlacementTransform, ParentPiece, ParentSnapId);
		if (!bResolvedTransform)
		{
			bResolvedTransform = ResolveFoundationPlacementTransform(PlacementTransform);
		}
	}
	else
	{
		bResolvedTransform = ResolveSnapPlacementTransform(PlacementTransform, ParentPiece, ParentSnapId);
	}

	if (!bResolvedTransform)
	{
		bCanPlaceAtCurrentPreview = false;
		DestroyPreviewActor();
		BP_OnPlacementPreviewChanged(false);
		return;
	}

	CurrentPlacementTransform = PlacementTransform;
	CurrentParentPiece = ParentPiece;
	CurrentParentSnapId = ParentSnapId;
	bCanPlaceAtCurrentPreview = CanPlaceAtTransform(CurrentPlacementTransform, *PieceDefinition, CurrentParentPiece, CurrentParentSnapId);
	UpdatePreviewActor(*PieceDefinition);
	BP_OnPlacementPreviewChanged(bCanPlaceAtCurrentPreview);
}

void UPW_PlayerBuildingPlacementComponent::UpdateDismantlePreview()
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (World == nullptr || Owner == nullptr)
	{
		return;
	}

	FVector ViewLocation;
	FVector ViewDirection;
	if (!ResolvePlacementView(ViewLocation, ViewDirection))
	{
		return;
	}

	const FVector TraceEnd = ViewLocation + ViewDirection.GetSafeNormal() * PlacementTraceDistance;
	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWBuildingDismantleTrace), false, Owner);
	
	APW_BuildingPieceActor* NewTarget = nullptr;
	if (World->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, PlacementTraceChannel, QueryParams))
	{
		NewTarget = Cast<APW_BuildingPieceActor>(HitResult.GetActor());
	}

	if (DismantleTargetPiece != NewTarget)
	{
		if (IsValid(DismantleTargetPiece))
		{
			DismantleTargetPiece->ClearPreviewMaterial();
		}
		
		DismantleTargetPiece = NewTarget;
		
		if (IsValid(DismantleTargetPiece))
		{
			UMaterialInterface* InvalidMaterial = nullptr;
			if (const FPW_BuildingPieceDefinition* Def = FindPieceDefinition(DismantleTargetPiece->GetPieceType()))
			{
				InvalidMaterial = Def->PreviewInvalidMaterial;
			}
			else if (PieceDefinitions.Num() > 0)
			{
				InvalidMaterial = PieceDefinitions[0].PreviewInvalidMaterial;
			}

			if (InvalidMaterial)
			{
				DismantleTargetPiece->ApplyPreviewMaterial(InvalidMaterial);
			}
		}
	}
}

bool UPW_PlayerBuildingPlacementComponent::ResolvePlacementView(FVector& OutViewLocation, FVector& OutViewDirection) const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const AController* Controller = OwnerPawn != nullptr ? OwnerPawn->GetController() : nullptr;
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController != nullptr)
	{
		FRotator ViewRotation;
		PlayerController->GetPlayerViewPoint(OutViewLocation, ViewRotation);
		OutViewDirection = ViewRotation.Vector();
		return true;
	}

	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}

	OutViewLocation = Owner->GetActorLocation();
	OutViewDirection = Owner->GetActorForwardVector();
	return true;
}

bool UPW_PlayerBuildingPlacementComponent::ResolveFoundationPlacementTransform(FTransform& OutPlacementTransform) const
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (World == nullptr || Owner == nullptr)
	{
		return false;
	}

	FVector ViewLocation;
	FVector ViewDirection;
	if (!ResolvePlacementView(ViewLocation, ViewDirection))
	{
		return false;
	}

	const FVector TraceEnd = ViewLocation + ViewDirection.GetSafeNormal() * PlacementTraceDistance;
	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWBuildingFoundationTrace), false, Owner);
	const bool bHit = World->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, PlacementTraceChannel, QueryParams);
	if (!bHit)
	{
		return false;
	}

	FRotator PlacementRotation = FRotator::ZeroRotator;
	PlacementRotation.Pitch = 0.0f;
	PlacementRotation.Roll = 0.0f;
	PlacementRotation.Yaw = FMath::UnwindDegrees(CurrentYawOffset);

	FVector PlacementLocation = HitResult.ImpactPoint;
	PlacementLocation.Z += CurrentFoundationHeightOffset;
	OutPlacementTransform = FTransform(PlacementRotation, PlacementLocation);
	return true;
}

bool UPW_PlayerBuildingPlacementComponent::ResolveSnapPlacementTransform(FTransform& OutPlacementTransform, APW_BuildingPieceActor*& OutParentPiece, FName& OutParentSnapId) const
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (World == nullptr || Owner == nullptr)
	{
		return false;
	}

	FVector ViewLocation;
	FVector ViewDirection;
	if (!ResolvePlacementView(ViewLocation, ViewDirection))
	{
		return false;
	}

	const FVector TraceEnd = ViewLocation + ViewDirection.GetSafeNormal() * PlacementTraceDistance;
	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWBuildingSnapTrace), false, Owner);
	const bool bHit = World->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, PlacementTraceChannel, QueryParams);
	if (!bHit)
	{
		return false;
	}

	APW_BuildingPieceActor* HitBuildingPiece = Cast<APW_BuildingPieceActor>(HitResult.GetActor());
	if (!IsValid(HitBuildingPiece))
	{
		return false;
	}

	const FVector QueryLocation = HitResult.ImpactPoint;
	APW_BuildingFoundationActor* HitFoundationPiece = Cast<APW_BuildingFoundationActor>(HitBuildingPiece);
	if (HitFoundationPiece && ResolveFoundationBasedSnap(HitFoundationPiece, QueryLocation, OutPlacementTransform, OutParentPiece, OutParentSnapId))
	{
		return true;
	}
	APW_BuildingFoundationActor* ParentFoundationPiece = Cast<APW_BuildingFoundationActor>(HitBuildingPiece->GetParentPiece());
	if (ParentFoundationPiece && ResolveFoundationBasedSnap(ParentFoundationPiece, QueryLocation, OutPlacementTransform, OutParentPiece, OutParentSnapId))
	{
		return true;
	}
	if (SelectedPieceType == EPW_BuildingPieceType::Foundation || SelectedPieceType == EPW_BuildingPieceType::Wall)
	{
		return false;
	}

	bool bFoundSnap = false;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	FPW_BuildingSnapPoint BestSnapPoint;
	FTransform BestParentSocketTransform = FTransform::Identity;
	TArray<APW_BuildingPieceActor*> CandidateParents;
	CandidateParents.Add(HitBuildingPiece);
	if (HitBuildingPiece->GetParentPiece())
	{
		CandidateParents.Add(HitBuildingPiece->GetParentPiece());
	}

	for (APW_BuildingPieceActor* CandidateParent : CandidateParents)
	{
		if (!IsValid(CandidateParent))
		{
			continue;
		}

		FPW_BuildingSnapPoint CandidateSnapPoint;
		FTransform CandidateSnapTransform;
		if (!CandidateParent->FindNearestCompatibleSnapPoint(SelectedPieceType, QueryLocation, CandidateSnapPoint, CandidateSnapTransform))
		{
			continue;
		}

		const FTransform ParentSocketTransform = CandidateSnapPoint.LocalTransform * CandidateParent->GetActorTransform();
		const FVector SnapQueryLocation = ParentSocketTransform.GetLocation();

		const float DistanceSquared = FVector::DistSquared(SnapQueryLocation, QueryLocation);
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			OutPlacementTransform = CandidateSnapTransform;
			OutParentPiece = CandidateParent;
			OutParentSnapId = CandidateSnapPoint.SnapId;
			BestSnapPoint = CandidateSnapPoint;
			BestParentSocketTransform = ParentSocketTransform;
			bFoundSnap = true;
		}
	}

	if (bFoundSnap)
	{
		FRotator PlacementRotation = OutPlacementTransform.GetRotation().Rotator();
		if (SelectedPieceType == EPW_BuildingPieceType::Foundation)
		{
			PlacementRotation.Pitch = 0.0f;
			PlacementRotation.Roll = 0.0f;
			PlacementRotation.Yaw = FMath::UnwindDegrees(PlacementRotation.Yaw);
		}
		else
		{
			PlacementRotation.Yaw = FMath::UnwindDegrees(PlacementRotation.Yaw + CurrentYawOffset);
			PlacementRotation.Yaw = FMath::GridSnap(PlacementRotation.Yaw, 90.0f);
		}
		OutPlacementTransform.SetRotation(PlacementRotation.Quaternion());

		if (SelectedPieceType != EPW_BuildingPieceType::Foundation)
		{
			const FVector ChildSocketOffset = OutPlacementTransform.GetRotation().RotateVector(BestSnapPoint.ChildLocalTransform.GetLocation());
			OutPlacementTransform.SetLocation(BestParentSocketTransform.GetLocation() - ChildSocketOffset);
		}
	}

	return bFoundSnap;
}

bool UPW_PlayerBuildingPlacementComponent::ResolveFoundationBasedSnap(
	APW_BuildingPieceActor* FoundationPiece,
	const FVector& QueryLocation,
	FTransform& OutPlacementTransform,
	APW_BuildingPieceActor*& OutParentPiece,
	FName& OutParentSnapId) const
{
	if (!IsValid(FoundationPiece)
		|| FoundationPiece->GetPieceType() != EPW_BuildingPieceType::Foundation
		|| (SelectedPieceType != EPW_BuildingPieceType::Foundation && SelectedPieceType != EPW_BuildingPieceType::Wall))
	{
		return false;
	}

	const APW_BuildingFoundationActor* FoundationActor = Cast<APW_BuildingFoundationActor>(FoundationPiece);
	if (FoundationActor == nullptr)
	{
		return false;
	}

	FName SnapId = NAME_None;
	if (!FoundationActor->ResolveSnapPlacement(SelectedPieceType, QueryLocation, CurrentYawOffset, OutPlacementTransform, SnapId)
		|| !FoundationActor->IsSnapPointAvailable(SnapId))
	{
		return false;
	}

	OutParentPiece = FoundationPiece;
	OutParentSnapId = SnapId;
	return true;
}

bool UPW_PlayerBuildingPlacementComponent::CanPlaceAtTransform(const FTransform& PlacementTransform, const FPW_BuildingPieceDefinition& PieceDefinition, APW_BuildingPieceActor* ParentPiece, FName ParentSnapId) const
{
	return ValidateServerPlacement(PieceDefinition, ParentPiece, ParentSnapId, PlacementTransform);
}

bool UPW_PlayerBuildingPlacementComponent::ValidateServerPlacement(const FPW_BuildingPieceDefinition& PieceDefinition, APW_BuildingPieceActor* ParentPiece, FName ParentSnapId, const FTransform& PlacementTransform) const
{
	UWorld* World = GetWorld();
	const AActor* Owner = GetOwner();
	if (World == nullptr || Owner == nullptr)
	{
		return false;
	}

	const float DistanceSquared = FVector::DistSquared(Owner->GetActorLocation(), PlacementTransform.GetLocation());
	if (DistanceSquared > FMath::Square(MaxPlacementDistanceFromOwner))
	{
		return false;
	}

	if (IsValid(ParentPiece) || PieceDefinition.PieceType != EPW_BuildingPieceType::Foundation)
	{
		if (!IsValid(ParentPiece) || ParentSnapId.IsNone())
		{
			return false;
		}

		if (const APW_BuildingFoundationActor* ParentFoundation = Cast<APW_BuildingFoundationActor>(ParentPiece))
		{
			FTransform ExpectedPlacementTransform;
			if (!ParentFoundation->ResolveSnapPlacementById(PieceDefinition.PieceType, ParentSnapId, 0.0f, ExpectedPlacementTransform)
				|| !ParentFoundation->IsSnapPointAvailable(ParentSnapId))
			{
				return false;
			}

			if (FVector::DistSquared(ExpectedPlacementTransform.GetLocation(), PlacementTransform.GetLocation()) > FMath::Square(25.0f))
			{
				return false;
			}

			return true;
		}

		FPW_BuildingSnapPoint ParentSnapPoint;
		if (!ParentPiece->FindSnapPoint(ParentSnapId, ParentSnapPoint)
			|| ParentSnapPoint.AcceptsPieceType != PieceDefinition.PieceType
			|| !ParentPiece->IsSnapPointAvailable(ParentSnapId))
		{
			return false;
		}

		const FTransform ParentSocketTransform = ParentSnapPoint.LocalTransform * ParentPiece->GetActorTransform();
		if (PieceDefinition.PieceType == EPW_BuildingPieceType::Foundation)
		{
			const FTransform ExpectedPlacementTransform = ParentSnapPoint.ChildLocalTransform.Inverse() * ParentSocketTransform;
			if (FVector::DistSquared(ExpectedPlacementTransform.GetLocation(), PlacementTransform.GetLocation()) > FMath::Square(25.0f))
			{
				return false;
			}
		}
		else
		{
			const FTransform ChildSocketTransform = ParentSnapPoint.ChildLocalTransform * PlacementTransform;
			if (FVector::DistSquared(ParentSocketTransform.GetLocation(), ChildSocketTransform.GetLocation()) > FMath::Square(25.0f))
			{
				return false;
			}
		}
	}
	else
	{
		FCollisionShape CollisionShape = FCollisionShape::MakeBox(PieceDefinition.PlacementOverlapExtent);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWBuildingOverlapTrace), false, Owner);
		if (PreviewActor)
		{
			QueryParams.AddIgnoredActor(PreviewActor);
		}

		TArray<FOverlapResult> OverlapResults;
		World->OverlapMultiByChannel(OverlapResults, PlacementTransform.GetLocation(), PlacementTransform.GetRotation(), PlacementOverlapChannel, CollisionShape, QueryParams);

		for (const FOverlapResult& Overlap : OverlapResults)
		{
			if (Cast<APW_BuildingPieceActor>(Overlap.GetActor()))
			{
				return false;
			}
		}
	}

	return true;
}

void UPW_PlayerBuildingPlacementComponent::UpdatePreviewActor(const FPW_BuildingPieceDefinition& PieceDefinition)
{
	UWorld* World = GetWorld();
	if (World == nullptr || World->IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	if (!PreviewActor || PreviewActor->GetClass() != PieceDefinition.PieceClass)
	{
		DestroyPreviewActor();

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PreviewActor = World->SpawnActor<APW_BuildingPieceActor>(PieceDefinition.PieceClass, CurrentPlacementTransform, SpawnParameters);
		if (PreviewActor)
		{
			PreviewActor->SetReplicates(false);
			PreviewActor->SetActorEnableCollision(false);
		}
	}

	if (!PreviewActor)
	{
		return;
	}

	PreviewActor->SetActorTransform(CurrentPlacementTransform);
	PreviewActor->ApplyPreviewMaterial(bCanPlaceAtCurrentPreview ? PieceDefinition.PreviewValidMaterial : PieceDefinition.PreviewInvalidMaterial);
}

void UPW_PlayerBuildingPlacementComponent::DestroyPreviewActor()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

APW_BuildingPieceActor* UPW_PlayerBuildingPlacementComponent::SpawnBuildingPieceAuthority(
	const FPW_BuildingPieceDefinition& PieceDefinition,
	const FPW_BuildingMaterialProfile& MaterialProfile,
	const FTransform& PlacementTransform,
	APW_BuildingPieceActor* ParentPiece,
	FName ParentSnapId)
{
	UWorld* World = GetWorld();
	if (World == nullptr || PieceDefinition.PieceClass == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APW_BuildingPieceActor* SpawnedPiece = World->SpawnActor<APW_BuildingPieceActor>(PieceDefinition.PieceClass, PlacementTransform, SpawnParameters);
	if (SpawnedPiece)
	{
		SpawnedPiece->InitializeBuildingPiece(
			PieceDefinition.PieceType,
			MaterialProfile.MaterialType,
			MaterialProfile.MaxDurability,
			MaterialProfile.MaterialOverride,
			ParentPiece,
			ParentSnapId);
		UE_LOG(LogTemp, Log, TEXT("PW_PlayerBuildingPlacementComponent spawned building piece: %s Class=%s Location=%s"),
			*GetNameSafe(SpawnedPiece),
			*GetNameSafe(PieceDefinition.PieceClass.Get()),
			*PlacementTransform.GetLocation().ToCompactString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PW_PlayerBuildingPlacementComponent failed to spawn building piece. Class=%s"),
			*GetNameSafe(PieceDefinition.PieceClass.Get()));
	}

	return SpawnedPiece;
}
