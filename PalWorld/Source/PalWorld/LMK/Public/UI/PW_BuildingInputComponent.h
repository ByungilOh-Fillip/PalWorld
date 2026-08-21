#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PW_BuildingInputComponent.generated.h"

class UPW_BuildingRadialMenuWidget;
class UPW_PlayerBasePlacementComponent;
class UPW_PlayerBuildingPlacementComponent;
class APlayerController;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_BuildingInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_BuildingInputComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Input")
	void ToggleBuildingMenu();

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Input")
	void ToggleDismantleMode();

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Input")
	void SetBuildingMenuVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Input")
	bool HandlePrimaryActionPressed();

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Input")
	bool HandleSecondaryActionPressed();

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Input")
	bool HandleWheelNext();

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Input")
	bool HandleWheelPrevious();

	UFUNCTION(BlueprintCallable, Category = "PW|Building|Input")
	bool HandleLookInput(const FVector2D& LookVector);

	UFUNCTION(BlueprintPure, Category = "PW|Building|Input")
	bool IsBuildingPlacementActive() const;

	UFUNCTION(BlueprintPure, Category = "PW|Building|Input")
	bool IsBuildingMenuVisible() const;

	UFUNCTION(BlueprintPure, Category = "PW|Building|Input")
	UPW_PlayerBuildingPlacementComponent* GetPlacementComponent() const;

	UFUNCTION(BlueprintPure, Category = "PW|Building|Input")
	UPW_PlayerBasePlacementComponent* GetBasePlacementComponent() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|UI")
	TSubclassOf<UPW_BuildingRadialMenuWidget> BuildingRadialMenuWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|UI")
	int32 BuildingMenuZOrder = 600;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Input")
	bool bAutoBindInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Building|Input")
	bool bConsumeAutoBoundInput = false;

private:
	bool bInputBound = false;
	uint64 LastToggleFrame = 0;
	FTimerHandle InputBindingRetryTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UPW_BuildingRadialMenuWidget> BuildingRadialMenuWidget;

	APlayerController* GetOwningPlayerController() const;
	UPW_BuildingRadialMenuWidget* GetOrCreateBuildingRadialMenu();
	void TryBindInputKeys();
	void BindInputKeys();

	void HandleAutoPrimaryPressed();
	void HandleAutoSecondaryPressed();
	void HandleAutoWheelNext();
	void HandleAutoWheelPrevious();
	void HandleAutoBasePlacementPressed();
};
