// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PWStaminaGaugeWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;
class UProgressBar;
class UPWPlayerStatComponent;

UCLASS(Blueprintable)
class PALWORLD_API UPWStaminaGaugeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|UI|Stamina")
	void InitializeWithStatComponent(UPWPlayerStatComponent* InStatComponent);

	UFUNCTION(BlueprintCallable, Category = "Player|UI|Stamina")
	void SetStaminaRatio(float InStaminaRatio);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Stamina")
	TObjectPtr<UImage> StaminaFillImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Player|UI|Stamina")
	TObjectPtr<UProgressBar> StaminaProgressBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|UI|Stamina")
	FName FillAmountParameterName = TEXT("FillAmount");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|UI|Stamina")
	float PreviewStaminaRatio = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|UI|Stamina")
	bool bHideWhenFull = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|UI|Stamina", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FullHiddenThreshold = 0.999f;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerStatComponent> BoundStatComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> StaminaMaterial;

	UFUNCTION()
	void HandleStaminaChanged(float CurrentStamina, float MaxStamina, float StaminaRatio);

	void UnbindStatComponent();
	void CacheStaminaMaterial();
	void UpdateGaugeVisibility(float StaminaRatio);
};
