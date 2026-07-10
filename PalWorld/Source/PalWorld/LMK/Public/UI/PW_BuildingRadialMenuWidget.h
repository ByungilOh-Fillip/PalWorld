#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BuildingTypes.h"
#include "Blueprint/UserWidget.h"
#include "PW_BuildingRadialMenuWidget.generated.h"

class UPW_PlayerBuildingPlacementComponent;
class UButton;
class UCanvasPanel;
class UImage;
class UTextBlock;
class UTexture2D;

UCLASS(Blueprintable)
class PALWORLD_API UPW_BuildingRadialMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "PW|Building|UI")
	void InitializeWithBuildingPlacementComponent(UPW_PlayerBuildingPlacementComponent* InPlacementComponent);

	UFUNCTION(BlueprintCallable, Category = "PW|Building|UI")
	void CycleMaterialPage();

	UFUNCTION(BlueprintCallable, Category = "PW|Building|UI")
	void SetCurrentMaterialType(EPW_BuildingMaterialType InMaterialType);

	UFUNCTION(BlueprintCallable, Category = "PW|Building|UI")
	void SelectFoundation();

	UFUNCTION(BlueprintCallable, Category = "PW|Building|UI")
	void SelectWall();

	UFUNCTION(BlueprintCallable, Category = "PW|Building|UI")
	void SelectRoof();

	UFUNCTION(BlueprintCallable, Category = "PW|Building|UI")
	void CancelBuilding();

	UFUNCTION(BlueprintPure, Category = "PW|Building|UI")
	EPW_BuildingMaterialType GetCurrentMaterialType() const { return CurrentMaterialType; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Building|UI", meta = (DisplayName = "On Material Page Changed"))
	void BP_OnMaterialPageChanged(EPW_BuildingMaterialType NewMaterialType);

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Building|UI", meta = (DisplayName = "On Building Piece Selected"))
	void BP_OnBuildingPieceSelected(EPW_BuildingPieceType PieceType, EPW_BuildingMaterialType MaterialType);

private:
	UPROPERTY(Transient)
	TObjectPtr<UPW_PlayerBuildingPlacementComponent> PlacementComponent;

	UPROPERTY(Transient)
	EPW_BuildingMaterialType CurrentMaterialType = EPW_BuildingMaterialType::Wood;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> DefaultRootCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UImage> DefaultRadialImage;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DefaultTitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DefaultMaterialText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DefaultSelectedText;

	void SelectPiece(EPW_BuildingPieceType PieceType);
	void BuildDefaultLayout();
	void UpdateDefaultTexts();
	FText GetMaterialDisplayText() const;
	FText GetMaterialStructureText() const;
	UButton* CreateDefaultButton(const FText& Label, const FVector2D& Position, const FVector2D& Size);

	UFUNCTION()
	void HandleFoundationClicked();

	UFUNCTION()
	void HandleWallClicked();

	UFUNCTION()
	void HandleRoofClicked();

	UFUNCTION()
	void HandleCycleMaterialClicked();
};
