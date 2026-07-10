#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BuildingTypes.h"
#include "Components/ActorComponent.h"
#include "PW_PlayerBuildingPlacementComponent.generated.h"

class APW_BuildingPieceActor;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_PlayerBuildingPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_PlayerBuildingPlacementComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Placement")
	void EnterPlacementMode(EPW_BuildingPieceType PieceType, EPW_BuildingMaterialType MaterialType);

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Placement")
	void CancelPlacement();

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Placement")
	void ConfirmPlacement();

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Placement")
	void RotatePreviewByWheel(float WheelDelta);

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Placement")
	void ApplyFoundationHeightMouseDelta(float MouseYDelta);

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Placement")
	bool TryConsumeLookInput(const FVector2D& LookVector);

	UFUNCTION(BlueprintPure, Category = "PW|Building|Placement")
	bool IsPlacementModeActive() const { return bPlacementModeActive; }

	UFUNCTION(BlueprintPure, Category = "PW|Building|Dismantle")
	bool IsDismantleModeActive() const { return bDismantleModeActive; }

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Dismantle")
	void SetDismantleModeActive(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Dismantle")
	void ConfirmDismantle();

	UFUNCTION(BlueprintPure, Category = "PW|Building|Placement")
	bool CanPlaceAtCurrentPreview() const { return bCanPlaceAtCurrentPreview; }

	UFUNCTION(BlueprintPure, Category = "PW|Building|Placement")
	EPW_BuildingPieceType GetSelectedPieceType() const { return SelectedPieceType; }

	UFUNCTION(BlueprintPure, Category = "PW|Building|Placement")
	EPW_BuildingMaterialType GetSelectedMaterialType() const { return SelectedMaterialType; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Placement", meta = (AllowPrivateAccess = "true"))
	TArray<FPW_BuildingPieceDefinition> PieceDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Placement")
	TArray<FPW_BuildingMaterialProfile> MaterialProfiles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Placement", meta = (ClampMin = "1.0"))
	float PlacementTraceDistance = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Placement", meta = (ClampMin = "1.0"))
	float MaxPlacementDistanceFromOwner = 8000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Placement")
	TEnumAsByte<ECollisionChannel> PlacementTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Placement")
	TEnumAsByte<ECollisionChannel> PlacementOverlapChannel = ECC_WorldStatic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Placement", meta = (ClampMin = "-1000.0", ClampMax = "0.0"))
	float MinFoundationHeightOffset = -100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Placement", meta = (ClampMin = "0.0"))
	float MaxFoundationHeightOffset = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Placement", meta = (ClampMin = "0.1"))
	float FoundationHeightMouseSensitivity = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Placement", meta = (ClampMin = "0.0"))
	float SnapSearchRadius = 220.0f;

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Building|Placement", meta = (DisplayName = "On Placement Mode Changed"))
	void BP_OnPlacementModeChanged(bool bActive);

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Building|Placement", meta = (DisplayName = "On Placement Preview Changed"))
	void BP_OnPlacementPreviewChanged(bool bCanPlace);

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Building|Dismantle", meta = (DisplayName = "On Dismantle Mode Changed"))
	void BP_OnDismantleModeChanged(bool bActive);

private:
	bool bPlacementModeActive = false;
	bool bDismantleModeActive = false;
	bool bCanPlaceAtCurrentPreview = false;
	float LastPolledMouseWheelAxis = 0.0f;
	float CurrentYawOffset = 0.0f;
	float CurrentFoundationHeightOffset = 0.0f;
	FTransform CurrentPlacementTransform = FTransform::Identity;
	EPW_BuildingPieceType SelectedPieceType = EPW_BuildingPieceType::Foundation;
	EPW_BuildingMaterialType SelectedMaterialType = EPW_BuildingMaterialType::Wood;
	FName CurrentParentSnapId = NAME_None;

	UPROPERTY(Transient)
	TObjectPtr<APW_BuildingPieceActor> CurrentParentPiece;

	UPROPERTY(Transient)
	TObjectPtr<APW_BuildingPieceActor> PreviewActor;

	UPROPERTY(Transient)
	TObjectPtr<APW_BuildingPieceActor> DismantleTargetPiece;

	UFUNCTION(Server, Reliable)
	void ServerRequestPlacePiece(
		EPW_BuildingPieceType PieceType,
		EPW_BuildingMaterialType MaterialType,
		FTransform PlacementTransform,
		APW_BuildingPieceActor* ParentPiece,
		FName ParentSnapId);

	UFUNCTION(Server, Reliable)
	void ServerRequestDismantlePiece(APW_BuildingPieceActor* TargetPiece);

	const FPW_BuildingPieceDefinition* FindPieceDefinition(EPW_BuildingPieceType PieceType) const;
	const FPW_BuildingMaterialProfile* FindMaterialProfile(EPW_BuildingMaterialType MaterialType) const;
	void EnsureDefaultMaterialProfiles();
	void EnsureDefaultPieceDefinitions();
	void SetPlacementModeActive(bool bNewActive);
	void UpdatePlacementPreview();
	void UpdateDismantlePreview();
	bool ResolvePlacementView(FVector& OutViewLocation, FVector& OutViewDirection) const;
	bool ResolveFoundationPlacementTransform(FTransform& OutPlacementTransform) const;
	bool ResolveSnapPlacementTransform(FTransform& OutPlacementTransform, APW_BuildingPieceActor*& OutParentPiece, FName& OutParentSnapId) const;
	bool ResolveFoundationBasedSnap(APW_BuildingPieceActor* FoundationPiece, const FVector& QueryLocation, FTransform& OutPlacementTransform, APW_BuildingPieceActor*& OutParentPiece, FName& OutParentSnapId) const;
	bool CanPlaceAtTransform(const FTransform& PlacementTransform, const FPW_BuildingPieceDefinition& PieceDefinition, APW_BuildingPieceActor* ParentPiece, FName ParentSnapId) const;
	bool ValidateServerPlacement(const FPW_BuildingPieceDefinition& PieceDefinition, APW_BuildingPieceActor* ParentPiece, FName ParentSnapId, const FTransform& PlacementTransform) const;
	void UpdatePreviewActor(const FPW_BuildingPieceDefinition& PieceDefinition);
	void DestroyPreviewActor();
	APW_BuildingPieceActor* SpawnBuildingPieceAuthority(const FPW_BuildingPieceDefinition& PieceDefinition, const FPW_BuildingMaterialProfile& MaterialProfile, const FTransform& PlacementTransform, APW_BuildingPieceActor* ParentPiece, FName ParentSnapId);
};
