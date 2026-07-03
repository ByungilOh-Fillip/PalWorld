#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PW_WorldMapControllerComponent.generated.h"

class UPW_WorldMapWidget;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_WorldMapControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_WorldMapControllerComponent();

	UFUNCTION(BlueprintCallable, Category = "PW|Map|UI")
	void ShowWorldMap();

	UFUNCTION(BlueprintCallable, Category = "PW|Map|UI")
	void HideWorldMap();

	UFUNCTION(BlueprintCallable, Category = "PW|Map|UI")
	void ToggleWorldMap();

	UFUNCTION(BlueprintPure, Category = "PW|Map|UI")
	bool IsWorldMapVisible() const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|UI")
	TSubclassOf<UPW_WorldMapWidget> WorldMapWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|UI")
	int32 WorldMapZOrder = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|UI")
	bool bApplyGameAndUIInputMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|UI")
	bool bRestoreGameOnlyInputModeOnHide = true;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPW_WorldMapWidget> WorldMapWidgetInstance;

	bool bPreviousShowMouseCursor = false;
	bool bHasAppliedWorldMapInputMode = false;

	class APlayerController* GetOwningPlayerController() const;
	void ConfigureWorldMapWidget(UPW_WorldMapWidget* Widget) const;
	void ApplyShowInputMode();
	void ApplyHideInputMode();
};
