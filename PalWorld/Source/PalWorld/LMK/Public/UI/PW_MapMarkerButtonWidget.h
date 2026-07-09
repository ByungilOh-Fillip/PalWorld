#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Map/PW_MapTypes.h"
#include "PW_MapMarkerButtonWidget.generated.h"

class UPW_WorldMapWidget;

UCLASS()
class PALWORLD_API UPW_MapMarkerButtonWidget : public UButton
{
	GENERATED_BODY()

public:
	UPW_MapMarkerButtonWidget(const FObjectInitializer& ObjectInitializer);

	void InitializeMarker(UPW_WorldMapWidget* InOwnerMapWidget, const FPW_MapMarker& InMarker);

	UFUNCTION(BlueprintPure, Category = "PW|Map|Marker")
	EPW_MapMarkerType GetMarkerType() const { return MarkerType; }

	UFUNCTION(BlueprintPure, Category = "PW|Map|Marker")
	FName GetMarkerId() const { return MarkerId; }

protected:
	UFUNCTION()
	void HandleClicked();

	UPROPERTY(Transient)
	TObjectPtr<UPW_WorldMapWidget> OwnerMapWidget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Map|Marker")
	EPW_MapMarkerType MarkerType = EPW_MapMarkerType::TeleportPoint;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Map|Marker")
	FName MarkerId = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Map|Marker")
	bool bCanTeleport = false;
};
