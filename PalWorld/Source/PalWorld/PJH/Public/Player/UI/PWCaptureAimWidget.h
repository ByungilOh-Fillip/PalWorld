// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PWCaptureAimWidget.generated.h"

class APWPlayerCharacter;
class UTextBlock;
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

	void BindCaptureComponent(UPWPlayerCaptureComponent* InCaptureComponent);
	void UnbindCaptureComponent();
	void RefreshCaptureAim();
	void SetTargetOnlyWidgetsVisibility(ESlateVisibility NewVisibility);

	UPROPERTY(Transient)
	TObjectPtr<APWPlayerCharacter> BoundPlayerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerCaptureComponent> BoundCaptureComponent;

	// WBP_CaptureAim의 최상위 패널. 이 이름으로 두면 C++이 표시/숨김을 직접 관리한다.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"), Category = "Player|UI|Capture")
	TObjectPtr<UWidget> Root_CaptureAim;

	// 조준선, 원형 UI처럼 타겟이 없어도 Q 조준 중에는 보이는 기본 파츠들.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"), Category = "Player|UI|Capture")
	TObjectPtr<UWidget> Panel_Reticle;

	// 펠에 조준됐을 때만 보이는 확률/이름 패널.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"), Category = "Player|UI|Capture")
	TObjectPtr<UWidget> Panel_TargetInfo;

	// 일반 Q 조준 때만 보이는 기존 조준 UI 묶음. 없으면 Panel_Reticle을 대신 숨긴다.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"), Category = "Player|UI|Capture")
	TObjectPtr<UWidget> Panel_AimOnly;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"), Category = "Player|UI|Capture")
	TObjectPtr<UTextBlock> Text_Chance;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"), Category = "Player|UI|Capture")
	TObjectPtr<UTextBlock> Text_CaptureChance;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"), Category = "Player|UI|Capture")
	TObjectPtr<UTextBlock> Text_TargetName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"), Category = "Player|UI|Capture")
	TObjectPtr<UTextBlock> Text_SphereCount;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UWidget>> TargetOnlyWidgets;
};
