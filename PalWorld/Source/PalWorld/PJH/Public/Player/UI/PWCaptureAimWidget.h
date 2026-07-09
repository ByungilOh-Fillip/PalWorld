// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PWCaptureAimWidget.generated.h"

class APWPlayerCharacter;
class UCanvasPanel;
class UImage;
class UTextBlock;
class UTexture2D;
class UPWPlayerCaptureComponent;
class UWidget;

UCLASS(Blueprintable)
class PALWORLD_API UPWCaptureAimWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player|UI|Capture")
	void InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter);

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleCaptureAimChanged();

	void BuildDefaultLayout();
	void BindCaptureComponent(UPWPlayerCaptureComponent* InCaptureComponent);
	void UnbindCaptureComponent();
	void RefreshCaptureAim();

	UImage* AddCaptureImage(const FName WidgetName, UTexture2D* Texture, const FVector2D Position, const FVector2D Size, float Opacity = 1.f);
	UTextBlock* AddCaptureText(const FName WidgetName, const FVector2D Position, const FVector2D Size, int32 FontSize);
	UTexture2D* LoadCaptureTexture(const TCHAR* TexturePath) const;

	UPROPERTY(Transient)
	TObjectPtr<APWPlayerCharacter> BoundPlayerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerCaptureComponent> BoundCaptureComponent;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> Text_Chance;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> Text_TargetName;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> Text_SphereCount;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UWidget>> TargetOnlyWidgets;

	bool bLayoutBuilt = false;
};
