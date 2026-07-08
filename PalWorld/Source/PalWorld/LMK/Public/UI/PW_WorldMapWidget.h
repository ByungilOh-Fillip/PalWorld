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
class UPW_MapMarkerButtonWidget;

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

	UFUNCTION(BlueprintCallable, Category = "PW|Map|Zoom")
	void SetMapZoom(float NewMapZoom);

	UFUNCTION(BlueprintPure, Category = "PW|Map|Zoom")
	float GetMapZoom() const { return CurrentMapZoom; }

	UFUNCTION(BlueprintCallable, Category = "PW|Map|Teleport")
	void SetTeleportSelectionEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintPure, Category = "PW|Map|Teleport")
	bool IsTeleportSelectionEnabled() const { return bTeleportSelectionEnabled; }

	UFUNCTION(BlueprintCallable, Category = "PW|Map|Teleport")
	bool SelectMapMarkerForTeleport(EPW_MapMarkerType MarkerType, FName MarkerId);

	UFUNCTION(BlueprintPure, Category = "PW|Map|Marker")
	UTexture2D* GetMarkerIcon(EPW_MapMarkerType MarkerType) const;

	UFUNCTION(BlueprintPure, Category = "PW|Map|Marker")
	FLinearColor GetMarkerTintColor(const FPW_MapMarker& Marker) const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "PW|Map", meta = (DisplayName = "On Map Data Refreshed"))
	void BP_OnMapDataRefreshed(
		const TArray<FPW_MapMarker>& Markers,
		const TArray<int32>& VisitedCellIndices,
		FVector2D PlayerMapUV,
		float RevealRadiusUV,
		int32 InGridWidth,
		int32 InGridHeight);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	TObjectPtr<UMaterialInterface> WorldMapMaterial;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map")
	TObjectPtr<UImage> MapBackgroundImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map|Zoom")
	TObjectPtr<UWidget> MapViewportRoot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map|Zoom")
	TObjectPtr<UWidget> MapZoomRoot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map|Exploration")
	TObjectPtr<UCanvasPanel> UnvisitedCellCanvas;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map|Exploration")
	TObjectPtr<UImage> VisitedDarkOverlayImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map|Exploration")
	TObjectPtr<UImage> InitialMapCoverImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Map|Marker")
	TObjectPtr<UCanvasPanel> MarkerCanvas;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker")
	bool bEnableBuiltInMarkerRendering = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker")
	bool bPreferRuntimeMarkerCanvas = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker")
	bool bDebugMapMarkerRendering = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker")
	TSubclassOf<UPW_MapMarkerButtonWidget> MarkerButtonWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker", meta = (ClampMin = "1.0"))
	FVector2D MarkerWidgetSize = FVector2D(32.0f, 32.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker", meta = (ClampMin = "1.0"))
	FVector2D MinimumMarkerWidgetSize = FVector2D(56.0f, 56.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker", meta = (ClampMin = "1.0"))
	FVector2D PlayerMarkerWidgetSize = FVector2D(72.0f, 72.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker")
	TObjectPtr<UTexture2D> TeleportMarkerIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker")
	TObjectPtr<UTexture2D> BaseCampMarkerIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker")
	FLinearColor ActiveTeleportMarkerColor = FLinearColor(0.0f, 0.35f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker")
	FLinearColor InactiveTeleportMarkerColor = FLinearColor(0.35f, 0.35f, 0.35f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker")
	FLinearColor ActiveBaseCampMarkerColor = FLinearColor(0.0f, 0.75f, 0.35f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Marker")
	FLinearColor InactiveBaseCampMarkerColor = FLinearColor(0.35f, 0.35f, 0.35f, 1.0f);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Map|Teleport")
	bool bTeleportSelectionEnabled = false;

private:
	FTimerHandle RefreshTimerHandle;
	FTimerHandle DeferredInitialRefreshTimerHandle;
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RuntimeMarkerCanvas;

	bool bIsDraggingMap = false;
	bool bHasCompletedInitialVisualRefresh = false;
	FVector2D LastDragScreenPosition = FVector2D::ZeroVector;
	FVector2D CurrentMapPanOffset = FVector2D::ZeroVector;
	TWeakObjectPtr<AActor> TrackedMapActor;

	void ApplyMapSettingsToSubsystem();
	void RefreshMapBackgroundImage();
	void ApplyMapZoom();
	void ApplyMapViewportSettings();
	void ApplyMapTransform();
	void RefreshBuiltInMapVisuals(const TArray<FPW_MapMarker>& Markers, const TArray<int32>& VisitedCellIndices, FVector2D PlayerMapUV, float RevealRadiusUV);
	void RefreshMapDataAfterLayout();
	void SetInitialMapCoverVisible(bool bVisible);
	void SetInitialVisualWidgetsVisible(bool bVisible);
	void ClampMapPanOffset();
	void SyncMapZoomRootToViewport();
	void SyncMapOverlaySlotsToBackground();
	void SyncCanvasSlotToBackground(UWidget* Widget, bool bMatchSize) const;
	void RefreshUnvisitedCells(const TArray<int32>& VisitedCellIndices, const FVector2D& MapSize);
	void RefreshBuiltInMapMarkers(const TArray<FPW_MapMarker>& Markers, const FVector2D& MapSize);
	UCanvasPanel* GetOrCreateMarkerCanvas();
	UCanvasPanel* GetMarkerCanvasParent() const;
	void PositionWidgetAtMapUV(UWidget* Widget, FVector2D MapUV, const FVector2D& MapSize) const;
	FVector2D GetMapVisualOrigin() const;
	FVector2D GetMapViewportSize() const;
	FVector2D GetMapVisualSize() const;
	bool IsRenderCellVisited(int32 RenderCellX, int32 RenderCellY, const TSet<int32>& VisitedCells) const;
};
