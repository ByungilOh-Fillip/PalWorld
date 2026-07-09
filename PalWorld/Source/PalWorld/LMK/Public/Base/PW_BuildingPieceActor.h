#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BuildingTypes.h"
#include "GameFramework/Actor.h"
#include "PW_BuildingPieceActor.generated.h"

class UMaterialInterface;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class PALWORLD_API APW_BuildingPieceActor : public AActor
{
	GENERATED_BODY()

public:
	APW_BuildingPieceActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Destroyed() override;

	UFUNCTION(BlueprintCallable, Category = "PW|Building")
	void InitializeBuildingPiece(
		EPW_BuildingPieceType InPieceType,
		EPW_BuildingMaterialType InMaterialType,
		float InMaxDurability,
		UMaterialInterface* InMaterialOverride,
		APW_BuildingPieceActor* InParentPiece,
		FName InParentSnapId);

	UFUNCTION(BlueprintCallable, Category = "PW|Building")
	void ApplyBuildingDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "PW|Building")
	void DestroyPiece();

	UFUNCTION(BlueprintPure, Category = "PW|Building")
	EPW_BuildingPieceType GetPieceType() const { return PieceType; }

	UFUNCTION(BlueprintPure, Category = "PW|Building")
	EPW_BuildingMaterialType GetMaterialType() const { return MaterialType; }

	UFUNCTION(BlueprintPure, Category = "PW|Building")
	float GetCurrentDurability() const { return CurrentDurability; }

	UFUNCTION(BlueprintPure, Category = "PW|Building")
	float GetMaxDurability() const { return MaxDurability; }

	UFUNCTION(BlueprintPure, Category = "PW|Building")
	APW_BuildingPieceActor* GetParentPiece() const { return ParentPiece; }

	UFUNCTION(BlueprintPure, Category = "PW|Building")
	FName GetParentSnapId() const { return ParentSnapId; }

	UFUNCTION(BlueprintPure, Category = "PW|Building")
	TArray<FPW_BuildingSnapPoint> GetSnapPoints() const { return SnapPoints; }

	UFUNCTION(BlueprintCallable, Category = "PW|Building")
	bool FindSnapPoint(FName SnapId, FPW_BuildingSnapPoint& OutSnapPoint) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Building")
	bool FindNearestCompatibleSnapPoint(
		EPW_BuildingPieceType DesiredPieceType,
		const FVector& QueryLocation,
		FPW_BuildingSnapPoint& OutSnapPoint,
		FTransform& OutSnapTransform) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Building")
	bool IsSnapPointAvailable(FName SnapId) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Building")
	void AddChildPiece(APW_BuildingPieceActor* ChildPiece);

	UFUNCTION(BlueprintCallable, Category = "PW|Building")
	void RemoveChildPiece(APW_BuildingPieceActor* ChildPiece);

	UFUNCTION(BlueprintCallable, Category = "PW|Building")
	void ApplyPreviewMaterial(UMaterialInterface* PreviewMaterial);

	UFUNCTION(BlueprintCallable, Category = "PW|Building")
	void ClearPreviewMaterial();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Building")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Building")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	EPW_BuildingPieceType DefaultPieceType = EPW_BuildingPieceType::Foundation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	TArray<FPW_BuildingSnapPoint> SnapPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	bool bUseCodeDefinedSnapPoints = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building")
	TArray<FPW_BuildingMaterialProfile> MaterialProfiles;

	UPROPERTY(ReplicatedUsing = OnRep_PieceState, BlueprintReadOnly, Category = "PW|Building")
	EPW_BuildingPieceType PieceType = EPW_BuildingPieceType::Foundation;

	UPROPERTY(ReplicatedUsing = OnRep_PieceState, BlueprintReadOnly, Category = "PW|Building")
	EPW_BuildingMaterialType MaterialType = EPW_BuildingMaterialType::Wood;

	UPROPERTY(ReplicatedUsing = OnRep_Durability, BlueprintReadOnly, Category = "PW|Building")
	float CurrentDurability = 500.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Durability, BlueprintReadOnly, Category = "PW|Building")
	float MaxDurability = 500.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PW|Building")
	TObjectPtr<APW_BuildingPieceActor> ParentPiece;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PW|Building")
	FName ParentSnapId = NAME_None;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PW|Building")
	TArray<TObjectPtr<APW_BuildingPieceActor>> ChildPieces;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_PieceState)
	TObjectPtr<UMaterialInterface> RuntimeMaterialOverride;

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Building", meta = (DisplayName = "On Building Piece State Changed"))
	void BP_OnBuildingPieceStateChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Building", meta = (DisplayName = "On Durability Changed"))
	void BP_OnDurabilityChanged();

	virtual void BuildCodeDefinedSnapPoints(TArray<FPW_BuildingSnapPoint>& OutSnapPoints) const;
	static FPW_BuildingSnapPoint MakeSnapPoint(
		FName SnapId,
		EPW_BuildingPieceType AcceptsPieceType,
		const FVector& LocalLocation,
		float LocalYaw,
		float SnapRadius,
		const FVector& ChildLocalLocation = FVector::ZeroVector,
		float ChildLocalYaw = 0.0f);

private:
	UPROPERTY(Transient)
	bool bIsDestroyingPiece = false;

	UFUNCTION()
	void OnRep_PieceState();

	UFUNCTION()
	void OnRep_Durability();

	void ApplyMaterialOverride();
	UMaterialInterface* ResolveMaterialOverride() const;
	void RefreshCodeDefinedSnapPoints();
	void SetMeshVisibleForGameplay();
	void DestroyChildren();
};
