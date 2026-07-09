#include "UI/PW_WorldMapWidget.h"

#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/Widget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Map/PW_MapSubsystem.h"
#include "Styling/SlateBrush.h"
#include "TimerManager.h"
#include "UI/PW_MapMarkerButtonWidget.h"
#include "UI/PW_WorldMapControllerComponent.h"

void UPW_WorldMapWidget::ConfigureMapWidget(
	const FString& InPlayerId,
	AActor* InTrackedMapActor)
{
	PlayerId = InPlayerId.IsEmpty() ? TEXT("LocalPlayer") : InPlayerId;
	TrackedMapActor = InTrackedMapActor;

	ApplyMapSettingsToSubsystem();
}

void UPW_WorldMapWidget::ConfigureMapWidgetWithSettings(
	const FString& InPlayerId,
	AActor* InTrackedMapActor,
	const FVector2D& InWorldMin,
	const FVector2D& InWorldMax,
	int32 InGridWidth,
	int32 InGridHeight,
	float InRevealRadius)
{
	WorldMin = InWorldMin;
	WorldMax = InWorldMax;
	GridWidth = FMath::Max(1, InGridWidth);
	GridHeight = FMath::Max(1, InGridHeight);
	RevealRadius = FMath::Max(0.0f, InRevealRadius);

	ConfigureMapWidget(InPlayerId, InTrackedMapActor);
}

void UPW_WorldMapWidget::RefreshMapData()
{
	UWorld* World = GetWorld();
	UPW_MapSubsystem* MapSubsystem = World != nullptr ? World->GetSubsystem<UPW_MapSubsystem>() : nullptr;
	if (MapSubsystem == nullptr)
	{
		return;
	}

	ApplyMapSettingsToSubsystem();

	TArray<FPW_MapMarker> Markers;
	MapSubsystem->GetMapMarkersForPlayer(PlayerId, GetOwningPlayer(), Markers);

	TArray<int32> VisitedCellIndices;
	MapSubsystem->GetVisitedCellIndices(PlayerId, VisitedCellIndices);

	FVector2D PlayerMapUV = FVector2D::ZeroVector;
	if (TrackedMapActor.IsValid())
	{
		const FVector ActorLocation = TrackedMapActor->GetActorLocation();
		MapSubsystem->RevealAroundLocation(PlayerId, ActorLocation);
		MapSubsystem->GetVisitedCellIndices(PlayerId, VisitedCellIndices);
		PlayerMapUV = MapSubsystem->WorldLocationToMapUV(ActorLocation);
	}
	else
	{
		MapSubsystem->GetLocalPlayerMapUV(PlayerMapUV);
	}

	RefreshBuiltInMapVisuals(Markers, VisitedCellIndices, PlayerMapUV, MapSubsystem->GetRevealRadiusUV());

	BP_OnMapDataRefreshed(
		Markers,
		VisitedCellIndices,
		PlayerMapUV,
		MapSubsystem->GetRevealRadiusUV(),
		GridWidth,
		GridHeight);
}

void UPW_WorldMapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	RefreshMapBackgroundImage();
	ApplyMapViewportSettings();
	SyncMapZoomRootToViewport();
	ApplyMapZoom();
	ApplyMapSettingsToSubsystem();
	SetInitialMapCoverVisible(true);
	SetInitialVisualWidgetsVisible(false);

	if (UWorld* World = GetWorld())
	{
		if (InitialRefreshDelaySeconds > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				DeferredInitialRefreshTimerHandle,
				this,
				&UPW_WorldMapWidget::RefreshMapDataAfterLayout,
				InitialRefreshDelaySeconds,
				false);
		}
		else
		{
			DeferredInitialRefreshTimerHandle = World->GetTimerManager().SetTimerForNextTick(this, &UPW_WorldMapWidget::RefreshMapDataAfterLayout);
		}
	}
	else
	{
		RefreshMapData();
	}

	if (bAutoRefresh)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				RefreshTimerHandle,
				this,
				&UPW_WorldMapWidget::RefreshMapData,
				RefreshIntervalSeconds,
				true);
		}
	}
}

void UPW_WorldMapWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
		World->GetTimerManager().ClearTimer(DeferredInitialRefreshTimerHandle);
	}

	Super::NativeDestruct();
}

FReply UPW_WorldMapWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	if (CurrentMapZoom <= MinMapZoom + UE_KINDA_SMALL_NUMBER)
	{
		return FReply::Handled();
	}

	bIsDraggingMap = true;
	LastDragScreenPosition = InMouseEvent.GetScreenSpacePosition();
	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply UPW_WorldMapWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	bIsDraggingMap = false;
	return FReply::Handled().ReleaseMouseCapture();
}

FReply UPW_WorldMapWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bIsDraggingMap)
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}

	const FVector2D CurrentScreenPosition = InMouseEvent.GetScreenSpacePosition();
	CurrentMapPanOffset += CurrentScreenPosition - LastDragScreenPosition;
	LastDragScreenPosition = CurrentScreenPosition;
	ClampMapPanOffset();
	ApplyMapTransform();
	return FReply::Handled();
}

FReply UPW_WorldMapWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseWheel(InGeometry, InMouseEvent);

	const float WheelDelta = InMouseEvent.GetWheelDelta();
	if (FMath::IsNearlyZero(WheelDelta))
	{
		return FReply::Unhandled();
	}

	SetMapZoom(CurrentMapZoom + WheelDelta * MouseWheelZoomStep);
	return FReply::Handled();
}

FReply UPW_WorldMapWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::M || InKeyEvent.GetKey() == EKeys::Escape)
	{
		APlayerController* OwningPlayer = GetOwningPlayer();
		UPW_WorldMapControllerComponent* WorldMapController = OwningPlayer != nullptr
			? OwningPlayer->FindComponentByClass<UPW_WorldMapControllerComponent>()
			: nullptr;
		if (WorldMapController != nullptr)
		{
			WorldMapController->HideWorldMap();
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPW_WorldMapWidget::SetMapZoom(float NewMapZoom)
{
	CurrentMapZoom = FMath::Clamp(NewMapZoom, MinMapZoom, MaxMapZoom);
	if (CurrentMapZoom <= MinMapZoom + UE_KINDA_SMALL_NUMBER)
	{
		CurrentMapPanOffset = FVector2D::ZeroVector;
		bIsDraggingMap = false;
	}
	else
	{
		ClampMapPanOffset();
	}

	ApplyMapZoom();
}

void UPW_WorldMapWidget::SetTeleportSelectionEnabled(bool bNewEnabled)
{
	bTeleportSelectionEnabled = bNewEnabled;
}

bool UPW_WorldMapWidget::SelectMapMarkerForTeleport(EPW_MapMarkerType MarkerType, FName MarkerId)
{
	if (!bTeleportSelectionEnabled || MarkerId.IsNone())
	{
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Yellow,
				FString::Printf(TEXT("[Map] Teleport marker ignored. SelectionMode=%s MarkerId=%s"),
					bTeleportSelectionEnabled ? TEXT("true") : TEXT("false"),
					*MarkerId.ToString()));
		}
		return false;
	}

	APlayerController* OwningPlayer = GetOwningPlayer();
	UPW_WorldMapControllerComponent* WorldMapController = OwningPlayer != nullptr
		? OwningPlayer->FindComponentByClass<UPW_WorldMapControllerComponent>()
		: nullptr;
	if (WorldMapController == nullptr)
	{
		if (GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Red,
				TEXT("[Map] Teleport marker ignored. WorldMapController missing."));
		}
		return false;
	}

	WorldMapController->RequestTeleportToMarker(MarkerType, MarkerId);
	return true;
}

UTexture2D* UPW_WorldMapWidget::GetMarkerIcon(EPW_MapMarkerType MarkerType) const
{
	if (MarkerType == EPW_MapMarkerType::BaseCamp)
	{
		return BaseCampMarkerIcon;
	}

	if (MarkerType == EPW_MapMarkerType::TeleportPoint)
	{
		return TeleportMarkerIcon;
	}

	return nullptr;
}

FLinearColor UPW_WorldMapWidget::GetMarkerTintColor(const FPW_MapMarker& Marker) const
{
	if (Marker.MarkerType == EPW_MapMarkerType::BaseCamp)
	{
		return Marker.bCanTeleport ? ActiveBaseCampMarkerColor : InactiveBaseCampMarkerColor;
	}

	return Marker.bCanTeleport ? ActiveTeleportMarkerColor : InactiveTeleportMarkerColor;
}

void UPW_WorldMapWidget::ApplyMapSettingsToSubsystem()
{
	UWorld* World = GetWorld();
	UPW_MapSubsystem* MapSubsystem = World != nullptr ? World->GetSubsystem<UPW_MapSubsystem>() : nullptr;
	if (MapSubsystem == nullptr)
	{
		return;
	}

	MapSubsystem->SetMapBounds(WorldMin, WorldMax);
	MapSubsystem->SetExplorationGridSize(GridWidth, GridHeight);
	MapSubsystem->SetRevealRadius(RevealRadius);
}

void UPW_WorldMapWidget::RefreshMapBackgroundImage()
{
	if (MapBackgroundImage == nullptr)
	{
		return;
	}

	if (WorldMapMaterial != nullptr)
	{
		MapBackgroundImage->SetBrushFromMaterial(WorldMapMaterial);
		return;
	}

	FSlateBrush EmptyBrush;
	MapBackgroundImage->SetBrush(EmptyBrush);
}

void UPW_WorldMapWidget::ApplyMapZoom()
{
	ApplyMapViewportSettings();
	SyncMapZoomRootToViewport();
	ClampMapPanOffset();
	ApplyMapTransform();
}

void UPW_WorldMapWidget::ApplyMapViewportSettings()
{
	if (MapViewportRoot != nullptr)
	{
		MapViewportRoot->SetClipping(EWidgetClipping::ClipToBounds);
	}
}

void UPW_WorldMapWidget::ApplyMapTransform()
{
	if (MapZoomRoot != nullptr)
	{
		MapZoomRoot->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		MapZoomRoot->SetRenderScale(FVector2D(CurrentMapZoom, CurrentMapZoom));
		MapZoomRoot->SetRenderTranslation(CurrentMapZoom > MinMapZoom + UE_KINDA_SMALL_NUMBER ? CurrentMapPanOffset : FVector2D::ZeroVector);
	}
}

void UPW_WorldMapWidget::RefreshBuiltInMapVisuals(const TArray<FPW_MapMarker>& Markers, const TArray<int32>& VisitedCellIndices, FVector2D PlayerMapUV, float RevealRadiusUV)
{
	SyncMapOverlaySlotsToBackground();

	const FVector2D MapSize = GetMapVisualSize();

	if (VisitedDarkOverlayImage != nullptr)
	{
		VisitedDarkOverlayImage->SetColorAndOpacity(VisitedDarkOverlayColor);
	}

	RefreshUnvisitedCells(VisitedCellIndices, MapSize);
	RefreshBuiltInMapMarkers(Markers, MapSize);
	PositionWidgetAtMapUV(PlayerMarkerWidget, PlayerMapUV, MapSize);
	PositionWidgetAtMapUV(CurrentAreaHighlightWidget, PlayerMapUV, MapSize);

	if (CurrentAreaHighlightWidget != nullptr)
	{
		const float HighlightDiameter = FMath::Max(MapSize.X, MapSize.Y) * RevealRadiusUV * 2.0f;
		if (UCanvasPanelSlot* HighlightSlot = Cast<UCanvasPanelSlot>(CurrentAreaHighlightWidget->Slot))
		{
			HighlightSlot->SetSize(FVector2D(HighlightDiameter, HighlightDiameter));
			HighlightSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			HighlightSlot->SetZOrder(40);
		}
	}

	if (UCanvasPanelSlot* PlayerMarkerSlot = PlayerMarkerWidget != nullptr ? Cast<UCanvasPanelSlot>(PlayerMarkerWidget->Slot) : nullptr)
	{
		PlayerMarkerSlot->SetSize(PlayerMarkerWidgetSize);
		PlayerMarkerSlot->SetZOrder(110);
	}

	if (!bHasCompletedInitialVisualRefresh)
	{
		bHasCompletedInitialVisualRefresh = true;
		SetInitialVisualWidgetsVisible(true);
		SetInitialMapCoverVisible(false);
	}
}

void UPW_WorldMapWidget::RefreshMapDataAfterLayout()
{
	ApplyMapZoom();
	RefreshMapData();
}

void UPW_WorldMapWidget::SetInitialMapCoverVisible(bool bVisible)
{
	if (InitialMapCoverImage != nullptr)
	{
		SyncCanvasSlotToBackground(InitialMapCoverImage, true);
		InitialMapCoverImage->SetColorAndOpacity(UnvisitedCellColor);
		InitialMapCoverImage->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UPW_WorldMapWidget::SetInitialVisualWidgetsVisible(bool bVisible)
{
	const ESlateVisibility NewVisibility = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;

	if (UnvisitedCellCanvas != nullptr)
	{
		UnvisitedCellCanvas->SetVisibility(NewVisibility);
	}

	if (VisitedDarkOverlayImage != nullptr)
	{
		VisitedDarkOverlayImage->SetVisibility(NewVisibility);
	}

	if (PlayerMarkerWidget != nullptr)
	{
		PlayerMarkerWidget->SetVisibility(NewVisibility);
	}

	if (CurrentAreaHighlightWidget != nullptr)
	{
		CurrentAreaHighlightWidget->SetVisibility(NewVisibility);
	}
}

void UPW_WorldMapWidget::ClampMapPanOffset()
{
	if (CurrentMapZoom <= MinMapZoom + UE_KINDA_SMALL_NUMBER)
	{
		CurrentMapPanOffset = FVector2D::ZeroVector;
		return;
	}

	const FVector2D ContentSize = GetMapVisualSize();
	const FVector2D ViewportSize = GetMapViewportSize();

	const FVector2D ScaledContentSize = ContentSize * CurrentMapZoom;
	const FVector2D MaxPan = FVector2D(
		FMath::Max(0.0f, (ScaledContentSize.X - ViewportSize.X) * 0.5f),
		FMath::Max(0.0f, (ScaledContentSize.Y - ViewportSize.Y) * 0.5f));

	CurrentMapPanOffset.X = FMath::Clamp(CurrentMapPanOffset.X, -MaxPan.X, MaxPan.X);
	CurrentMapPanOffset.Y = FMath::Clamp(CurrentMapPanOffset.Y, -MaxPan.Y, MaxPan.Y);
}

void UPW_WorldMapWidget::SyncMapZoomRootToViewport()
{
	if (MapViewportRoot == nullptr || MapZoomRoot == nullptr)
	{
		return;
	}

	UCanvasPanelSlot* ZoomRootSlot = Cast<UCanvasPanelSlot>(MapZoomRoot->Slot);
	if (ZoomRootSlot == nullptr)
	{
		return;
	}

	ZoomRootSlot->SetAnchors(FAnchors(0.0f, 0.0f));
	ZoomRootSlot->SetAlignment(FVector2D::ZeroVector);
	ZoomRootSlot->SetPosition(FVector2D::ZeroVector);

	const FVector2D ViewportSize = GetMapViewportSize();
	if (ViewportSize.X > 1.0f && ViewportSize.Y > 1.0f)
	{
		ZoomRootSlot->SetSize(ViewportSize);
	}
}

void UPW_WorldMapWidget::SyncMapOverlaySlotsToBackground()
{
	SyncCanvasSlotToBackground(UnvisitedCellCanvas, true);
	SyncCanvasSlotToBackground(VisitedDarkOverlayImage, true);
	SyncCanvasSlotToBackground(InitialMapCoverImage, true);
	SyncCanvasSlotToBackground(GetOrCreateMarkerCanvas(), true);

	if (UnvisitedCellCanvas != nullptr)
	{
		UnvisitedCellCanvas->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* UnvisitedSlot = Cast<UCanvasPanelSlot>(UnvisitedCellCanvas->Slot))
		{
			UnvisitedSlot->SetZOrder(20);
		}
	}

	if (VisitedDarkOverlayImage != nullptr)
	{
		VisitedDarkOverlayImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* VisitedOverlaySlot = Cast<UCanvasPanelSlot>(VisitedDarkOverlayImage->Slot))
		{
			VisitedOverlaySlot->SetZOrder(10);
		}
	}

	if (InitialMapCoverImage != nullptr && InitialMapCoverImage->GetVisibility() != ESlateVisibility::Collapsed)
	{
		InitialMapCoverImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* InitialCoverSlot = Cast<UCanvasPanelSlot>(InitialMapCoverImage->Slot))
		{
			InitialCoverSlot->SetZOrder(30);
		}
	}

	if (UCanvasPanel* TargetMarkerCanvas = GetOrCreateMarkerCanvas())
	{
		TargetMarkerCanvas->SetVisibility(ESlateVisibility::Visible);
		if (UCanvasPanelSlot* MarkerCanvasSlot = Cast<UCanvasPanelSlot>(TargetMarkerCanvas->Slot))
		{
			MarkerCanvasSlot->SetZOrder(100);
		}
	}
}

void UPW_WorldMapWidget::SyncCanvasSlotToBackground(UWidget* Widget, bool bMatchSize) const
{
	if (Widget == nullptr || MapBackgroundImage == nullptr || Widget == MapBackgroundImage)
	{
		return;
	}

	const UCanvasPanelSlot* BackgroundSlot = Cast<UCanvasPanelSlot>(MapBackgroundImage->Slot);
	UCanvasPanelSlot* TargetSlot = Cast<UCanvasPanelSlot>(Widget->Slot);
	if (BackgroundSlot == nullptr || TargetSlot == nullptr)
	{
		return;
	}

	TargetSlot->SetAnchors(BackgroundSlot->GetAnchors());
	TargetSlot->SetAlignment(BackgroundSlot->GetAlignment());
	TargetSlot->SetPosition(BackgroundSlot->GetPosition());
	if (bMatchSize)
	{
		TargetSlot->SetSize(GetMapVisualSize());
	}
}

void UPW_WorldMapWidget::RefreshUnvisitedCells(const TArray<int32>& VisitedCellIndices, const FVector2D& MapSize)
{
	if (UnvisitedCellCanvas == nullptr || ExplorationRenderGridWidth <= 0 || ExplorationRenderGridHeight <= 0)
	{
		return;
	}

	TSet<int32> VisitedCells;
	for (const int32 CellIndex : VisitedCellIndices)
	{
		VisitedCells.Add(CellIndex);
	}

	UnvisitedCellCanvas->ClearChildren();

	const FVector2D CellSize(
		MapSize.X / ExplorationRenderGridWidth,
		MapSize.Y / ExplorationRenderGridHeight);

	for (int32 CellY = 0; CellY < ExplorationRenderGridHeight; ++CellY)
	{
		for (int32 CellX = 0; CellX < ExplorationRenderGridWidth; ++CellX)
		{
			if (IsRenderCellVisited(CellX, CellY, VisitedCells))
			{
				continue;
			}

			UBorder* CellBorder = NewObject<UBorder>(UnvisitedCellCanvas);
			if (CellBorder == nullptr)
			{
				continue;
			}

			CellBorder->SetBrushColor(UnvisitedCellColor);
			CellBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
			UCanvasPanelSlot* CellSlot = UnvisitedCellCanvas->AddChildToCanvas(CellBorder);
			if (CellSlot != nullptr)
			{
				CellSlot->SetAutoSize(false);
				CellSlot->SetPosition(FVector2D(CellX * CellSize.X, CellY * CellSize.Y));
				CellSlot->SetSize(CellSize + FVector2D(1.0f, 1.0f));
			}
		}
	}
}

void UPW_WorldMapWidget::RefreshBuiltInMapMarkers(const TArray<FPW_MapMarker>& Markers, const FVector2D& MapSize)
{
	UCanvasPanel* TargetMarkerCanvas = GetOrCreateMarkerCanvas();
	if (!bEnableBuiltInMarkerRendering || TargetMarkerCanvas == nullptr)
	{
		return;
	}

	TargetMarkerCanvas->ClearChildren();
	TargetMarkerCanvas->SetVisibility(ESlateVisibility::Visible);

	TSubclassOf<UPW_MapMarkerButtonWidget> ResolvedMarkerButtonClass = MarkerButtonWidgetClass;
	if (ResolvedMarkerButtonClass == nullptr)
	{
		ResolvedMarkerButtonClass = UPW_MapMarkerButtonWidget::StaticClass();
	}

	for (const FPW_MapMarker& Marker : Markers)
	{
		if (Marker.MarkerType == EPW_MapMarkerType::Player)
		{
			continue;
		}

		UPW_MapMarkerButtonWidget* MarkerButton = NewObject<UPW_MapMarkerButtonWidget>(TargetMarkerCanvas, ResolvedMarkerButtonClass);
		if (MarkerButton == nullptr)
		{
			continue;
		}

		MarkerButton->InitializeMarker(this, Marker);
		UCanvasPanelSlot* MarkerSlot = TargetMarkerCanvas->AddChildToCanvas(MarkerButton);
		if (MarkerSlot != nullptr)
		{
			const FVector2D ResolvedMarkerSize(
				FMath::Max(MarkerWidgetSize.X, MinimumMarkerWidgetSize.X),
				FMath::Max(MarkerWidgetSize.Y, MinimumMarkerWidgetSize.Y));
			MarkerSlot->SetAutoSize(false);
			MarkerSlot->SetSize(ResolvedMarkerSize);
			MarkerSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			MarkerSlot->SetPosition(FVector2D(Marker.MapUV.X * MapSize.X, Marker.MapUV.Y * MapSize.Y));
			MarkerSlot->SetZOrder(1);
		}
	}

	if (bDebugMapMarkerRendering && GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.0f,
			FColor::Cyan,
			FString::Printf(TEXT("[Map] MarkerCanvas=%s InputMarkers=%d Rendered=%d"),
				*GetNameSafe(TargetMarkerCanvas),
				Markers.Num(),
				TargetMarkerCanvas->GetChildrenCount()));
	}
}

UCanvasPanel* UPW_WorldMapWidget::GetOrCreateMarkerCanvas()
{
	if (!bPreferRuntimeMarkerCanvas && MarkerCanvas != nullptr)
	{
		return MarkerCanvas;
	}

	if (RuntimeMarkerCanvas != nullptr)
	{
		return RuntimeMarkerCanvas;
	}

	UCanvasPanel* MarkerParentCanvas = GetMarkerCanvasParent();
	if (MarkerParentCanvas == nullptr)
	{
		return MarkerCanvas;
	}

	RuntimeMarkerCanvas = NewObject<UCanvasPanel>(this);
	if (RuntimeMarkerCanvas == nullptr)
	{
		return nullptr;
	}

	UCanvasPanelSlot* MarkerCanvasSlot = MarkerParentCanvas->AddChildToCanvas(RuntimeMarkerCanvas);
	if (MarkerCanvasSlot != nullptr)
	{
		MarkerCanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		MarkerCanvasSlot->SetAlignment(FVector2D::ZeroVector);
		MarkerCanvasSlot->SetPosition(FVector2D::ZeroVector);
		MarkerCanvasSlot->SetSize(GetMapVisualSize());
		MarkerCanvasSlot->SetZOrder(100);
	}

	return RuntimeMarkerCanvas;
}

UCanvasPanel* UPW_WorldMapWidget::GetMarkerCanvasParent() const
{
	if (MapBackgroundImage != nullptr)
	{
		if (UCanvasPanel* BackgroundParentCanvas = Cast<UCanvasPanel>(MapBackgroundImage->GetParent()))
		{
			return BackgroundParentCanvas;
		}
	}

	if (UnvisitedCellCanvas != nullptr)
	{
		if (UCanvasPanel* UnvisitedParentCanvas = Cast<UCanvasPanel>(UnvisitedCellCanvas->GetParent()))
		{
			return UnvisitedParentCanvas;
		}
	}

	if (VisitedDarkOverlayImage != nullptr)
	{
		if (UCanvasPanel* VisitedOverlayParentCanvas = Cast<UCanvasPanel>(VisitedDarkOverlayImage->GetParent()))
		{
			return VisitedOverlayParentCanvas;
		}
	}

	if (InitialMapCoverImage != nullptr)
	{
		if (UCanvasPanel* InitialCoverParentCanvas = Cast<UCanvasPanel>(InitialMapCoverImage->GetParent()))
		{
			return InitialCoverParentCanvas;
		}
	}

	return Cast<UCanvasPanel>(MapZoomRoot);
}

void UPW_WorldMapWidget::PositionWidgetAtMapUV(UWidget* Widget, FVector2D MapUV, const FVector2D& MapSize) const
{
	if (Widget == nullptr)
	{
		return;
	}

	FVector2D Position = GetMapVisualOrigin() + FVector2D(MapUV.X * MapSize.X, MapUV.Y * MapSize.Y);
	if (MapBackgroundImage != nullptr && Widget->GetParent() != nullptr)
	{
		const FGeometry& BackgroundGeometry = MapBackgroundImage->GetCachedGeometry();
		const FGeometry& ParentGeometry = Widget->GetParent()->GetCachedGeometry();
		const FVector2D BackgroundSize = BackgroundGeometry.GetLocalSize();
		if (BackgroundSize.X > 1.0f && BackgroundSize.Y > 1.0f)
		{
			const FVector2D BackgroundLocalPosition(MapUV.X * BackgroundSize.X, MapUV.Y * BackgroundSize.Y);
			Position = ParentGeometry.AbsoluteToLocal(BackgroundGeometry.LocalToAbsolute(BackgroundLocalPosition));
		}
	}

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(Position);
		return;
	}

	Widget->SetRenderTranslation(Position);
}

FVector2D UPW_WorldMapWidget::GetMapVisualOrigin() const
{
	if (MapBackgroundImage != nullptr)
	{
		if (const UCanvasPanelSlot* BackgroundSlot = Cast<UCanvasPanelSlot>(MapBackgroundImage->Slot))
		{
			const FVector2D SlotSize = GetMapVisualSize();
			return BackgroundSlot->GetPosition() - BackgroundSlot->GetAlignment() * SlotSize;
		}
	}

	return FVector2D::ZeroVector;
}

FVector2D UPW_WorldMapWidget::GetMapViewportSize() const
{
	if (MapViewportRoot != nullptr)
	{
		if (const UCanvasPanelSlot* ViewportSlot = Cast<UCanvasPanelSlot>(MapViewportRoot->Slot))
		{
			const FVector2D SlotSize = ViewportSlot->GetSize();
			if (SlotSize.X > 1.0f && SlotSize.Y > 1.0f)
			{
				return SlotSize;
			}
		}

		const FVector2D LocalSize = MapViewportRoot->GetCachedGeometry().GetLocalSize();
		if (LocalSize.X > 1.0f && LocalSize.Y > 1.0f)
		{
			return LocalSize;
		}
	}

	return GetMapVisualSize();
}

FVector2D UPW_WorldMapWidget::GetMapVisualSize() const
{
	if (MapBackgroundImage != nullptr)
	{
		if (const UCanvasPanelSlot* BackgroundSlot = Cast<UCanvasPanelSlot>(MapBackgroundImage->Slot))
		{
			const FVector2D SlotSize = BackgroundSlot->GetSize();
			if (SlotSize.X > 1.0f && SlotSize.Y > 1.0f)
			{
				return SlotSize;
			}
		}

		const FVector2D LocalSize = MapBackgroundImage->GetCachedGeometry().GetLocalSize();
		if (LocalSize.X > 1.0f && LocalSize.Y > 1.0f)
		{
			return LocalSize;
		}
	}

	if (MapZoomRoot != nullptr)
	{
		const FVector2D LocalSize = MapZoomRoot->GetCachedGeometry().GetLocalSize();
		if (LocalSize.X > 1.0f && LocalSize.Y > 1.0f)
		{
			return LocalSize;
		}
	}

	return FallbackMapWidgetSize;
}

bool UPW_WorldMapWidget::IsRenderCellVisited(int32 RenderCellX, int32 RenderCellY, const TSet<int32>& VisitedCells) const
{
	const int32 StartCellX = FMath::FloorToInt(static_cast<float>(RenderCellX) * GridWidth / ExplorationRenderGridWidth);
	const int32 EndCellX = FMath::CeilToInt(static_cast<float>(RenderCellX + 1) * GridWidth / ExplorationRenderGridWidth);
	const int32 StartCellY = FMath::FloorToInt(static_cast<float>(RenderCellY) * GridHeight / ExplorationRenderGridHeight);
	const int32 EndCellY = FMath::CeilToInt(static_cast<float>(RenderCellY + 1) * GridHeight / ExplorationRenderGridHeight);

	for (int32 CellY = StartCellY; CellY < EndCellY; ++CellY)
	{
		for (int32 CellX = StartCellX; CellX < EndCellX; ++CellX)
		{
			const int32 CellIndex = CellY * GridWidth + CellX;
			if (VisitedCells.Contains(CellIndex))
			{
				return true;
			}
		}
	}

	return false;
}
