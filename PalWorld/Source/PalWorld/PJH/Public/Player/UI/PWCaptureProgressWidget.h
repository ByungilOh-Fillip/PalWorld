// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PWCaptureProgressWidget.generated.h"

class UImage;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTextBlock;

UCLASS(Blueprintable)
class PALWORLD_API UPWCaptureProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPWCaptureProgressWidget(const FObjectInitializer& ObjectInitializer);

	void SetCaptureProgress(float DisplayChance, bool bCapturing);
	void SetCaptureVisible(bool bVisible);

protected:
	virtual void NativeConstruct() override;

private:
	void InitializeProgressMaterial();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"), Category = "Player|UI|Capture")
	TObjectPtr<UImage> Image_CaptureProgress;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"), Category = "Player|UI|Capture")
	TObjectPtr<UTextBlock> Text_CaptureChance;

	UPROPERTY(EditDefaultsOnly, Category = "Player|UI|Capture")
	TObjectPtr<UMaterialInterface> ProgressMaterial = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ProgressMaterialInstance = nullptr;
};
