#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "Map/PW_MapTypes.h"
#include "PW_WorldMapWidget.generated.h"

class UImage;
class UCanvasPanel;
class UMaterialInterface;
class UTexture2D;
class UWidget;
class AActor;

UCLASS(Blueprintable)
class PALWORLD_API UPW_WorldMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PW|Map")
	void ConfigureMapWidget(
		const FString& InPlayerId,
		AActor* InTrackedMapActor);

	UFUNCTION(BlueprintCallable, Category = "PW|Map")
	void ConfigureMapWidgetWithSettings(
		const FString& InPlayerId,
		AActor* InTrackedMapActor,
		const FVector2D& InWorldMin,
		const FVector2D& InWorldMax,
		int32 InGridWidth,
		int32 InGridHeight,
		float InRevealRadius);

	UFUNCTION(BlueprintCallable, Category = "PW|Map")
	void RefreshMapData();

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	UTexture2D* GetWorldMapTexture() const { return WorldMapTexture; }

	UFUNCTION(BlueprintCallable, Category = "PW|Map|Zoom")
	void SetMapZoom(float NewMapZoom);

	UFUNCTION(BlueprintPure, Category = "PW|Map|Zoom")
	float GetMapZoom() const { return CurrentMapZoom; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Map", meta = (DisplayName = "On Map Data Refreshed"))
	void BP_OnMapDataRefreshed(
		const TArray<FPW_MapMarker>& Markers,
		const TArray<int32>& VisitedCellIndices,
		FVector2D PlayerMapUV,
		float RevealRadiusUV,
		int32 InGridWidth,
		int32 InGridHeight);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	TObjectPtr<UTexture2D> WorldMapTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	TObjectPtr<UMaterialInterface> WorldMapMaterial;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map")
	TObjectPtr<UImage> MapBackgroundImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map|Zoom")
	TObjectPtr<UWidget> MapZoomRoot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map|Exploration")
	TObjectPtr<UCanvasPanel> UnvisitedCellCanvas;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map|Exploration")
	TObjectPtr<UImage> VisitedDarkOverlayImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map|Exploration")
	TObjectPtr<UImage> InitialMapCoverImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map|Marker")
	TObjectPtr<UWidget> PlayerMarkerWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map|Marker")
	TObjectPtr<UWidget> CurrentAreaHighlightWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	FVector2D WorldMin = FVector2D(-50000.0f, -50000.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	FVector2D WorldMax = FVector2D(50000.0f, 50000.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map", meta = (ClampMin = "1"))
	int32 GridWidth = 128;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map", meta = (ClampMin = "1"))
	int32 GridHeight = 128;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map", meta = (ClampMin = "0.0"))
	float RevealRadius = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	FString PlayerId = TEXT("LocalPlayer");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	bool bAutoRefresh = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map", meta = (ClampMin = "0.05"))
	float RefreshIntervalSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map", meta = (ClampMin = "0.0"))
	float InitialRefreshDelaySeconds = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Zoom", meta = (ClampMin = "0.1"))
	float MinMapZoom = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Zoom", meta = (ClampMin = "0.1"))
	float MaxMapZoom = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Zoom", meta = (ClampMin = "0.01"))
	float MouseWheelZoomStep = 0.15f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Map|Zoom")
	float CurrentMapZoom = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Exploration", meta = (ClampMin = "1"))
	int32 ExplorationRenderGridWidth = 64;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Exploration", meta = (ClampMin = "1"))
	int32 ExplorationRenderGridHeight = 64;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Exploration")
	FLinearColor UnvisitedCellColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Exploration")
	FLinearColor VisitedDarkOverlayColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker")
	FVector2D FallbackMapWidgetSize = FVector2D(1024.0f, 1024.0f);

private:
	FTimerHandle RefreshTimerHandle;
	FTimerHandle DeferredInitialRefreshTimerHandle;
	bool bIsDraggingMap = false;
	bool bHasCompletedInitialVisualRefresh = false;
	FVector2D LastDragScreenPosition = FVector2D::ZeroVector;
	FVector2D CurrentMapPanOffset = FVector2D::ZeroVector;
	TWeakObjectPtr<AActor> TrackedMapActor;

	void ApplyMapSettingsToSubsystem();
	void RefreshMapBackgroundImage();
	void ApplyMapZoom();
	void ApplyMapTransform();
	void RefreshBuiltInMapVisuals(const TArray<int32>& VisitedCellIndices, FVector2D PlayerMapUV, float RevealRadiusUV);
	void RefreshMapDataAfterLayout();
	void SetInitialMapCoverVisible(bool bVisible);
	void SetInitialVisualWidgetsVisible(bool bVisible);
	void SyncMapOverlaySlotsToBackground();
	void SyncCanvasSlotToBackground(UWidget* Widget, bool bMatchSize) const;
	void RefreshUnvisitedCells(const TArray<int32>& VisitedCellIndices, const FVector2D& MapSize);
	void PositionWidgetAtMapUV(UWidget* Widget, FVector2D MapUV, const FVector2D& MapSize) const;
	FVector2D GetMapVisualOrigin() const;
	FVector2D GetMapVisualSize() const;
	bool IsRenderCellVisited(int32 RenderCellX, int32 RenderCellY, const TSet<int32>& VisitedCells) const;
};
