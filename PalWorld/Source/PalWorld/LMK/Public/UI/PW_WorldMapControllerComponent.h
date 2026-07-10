#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Map/PW_MapTypes.h"
#include "PW_WorldMapControllerComponent.generated.h"

class UPW_WorldMapWidget;
class UPW_MapSubsystem;
class APW_TeleportPointActor;
class APW_BaseCampActor;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_WorldMapControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_WorldMapControllerComponent();

	UFUNCTION(BlueprintCallable, Category = "PW|Map|UI")
	void ShowWorldMap();

	UFUNCTION(BlueprintCallable, Category = "PW|Map|UI")
	void ShowTeleportMap();

	UFUNCTION(BlueprintCallable, Category = "PW|Map|UI")
	void HideWorldMap();

	UFUNCTION(BlueprintCallable, Category = "PW|Map|UI")
	void ToggleWorldMap();

	UFUNCTION(BlueprintPure, Category = "PW|Map|UI")
	bool IsWorldMapVisible() const;

	UFUNCTION(BlueprintCallable, Category = "PW|Map")
	void ConfigureMapSubsystem();

	UFUNCTION(BlueprintCallable, Category = "PW|Map")
	void RevealControlledPawnLocation();

	UFUNCTION(BlueprintCallable, Category = "PW|Map|Teleport")
	void RequestTeleportToMarker(EPW_MapMarkerType MarkerType, FName MarkerId);

	void SetActiveTeleportSource(APW_TeleportPointActor* TeleportSource);
	void SetActiveBaseCampTeleportSource(APW_BaseCampActor* BaseCampSource);

	UFUNCTION(Client, Reliable)
	void ClientShowTeleportMap();

	UFUNCTION(BlueprintPure, Category = "PW|Map")
	FString GetResolvedPlayerId() const;

	UFUNCTION(BlueprintPure, Category = "PW|Map|Save")
	FPW_MapExplorationSaveData MakeExplorationSaveData() const;

	UFUNCTION(BlueprintCallable, Category = "PW|Map|Save")
	bool ApplyExplorationSaveData(const FPW_MapExplorationSaveData& SaveData);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|UI")
	TSubclassOf<UPW_WorldMapWidget> WorldMapWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|UI")
	int32 WorldMapZOrder = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|UI")
	bool bApplyGameAndUIInputMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|UI")
	bool bUseUIOnlyInputModeWhenMapVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|UI")
	bool bRestoreGameOnlyInputModeOnHide = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	FString PlayerId = TEXT("LocalPlayer");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	bool bApplyMapSettingsOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map")
	bool bAutoReveal = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map", meta = (ClampMin = "0.05"))
	float RevealUpdateIntervalSeconds = 0.5f;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport")
	FVector TeleportArrivalOffset = FVector(0.0f, 0.0f, 120.0f);

private:
	UPROPERTY(Transient)
	TObjectPtr<UPW_WorldMapWidget> WorldMapWidgetInstance;

	UPROPERTY(Transient)
	TObjectPtr<APW_TeleportPointActor> ActiveTeleportSource;

	UPROPERTY(Transient)
	TObjectPtr<APW_BaseCampActor> ActiveBaseCampTeleportSource;

	FTimerHandle RevealTimerHandle;
	bool bPreviousShowMouseCursor = false;
	bool bHasAppliedWorldMapInputMode = false;
	bool bTeleportSelectionMode = false;

	class APlayerController* GetOwningPlayerController() const;
	class APawn* GetControlledPawn() const;
	UPW_MapSubsystem* GetMapSubsystem() const;
	void ConfigureWorldMapWidget(UPW_WorldMapWidget* Widget) const;
	void ShowWorldMapInternal(bool bEnableTeleportSelection);
	void ApplyShowInputMode();
	void ApplyHideInputMode();
	bool CanUseActiveTeleportSource() const;

	UFUNCTION(Server, Reliable)
	void ServerRequestTeleportToMarker(EPW_MapMarkerType MarkerType, FName MarkerId);
};
