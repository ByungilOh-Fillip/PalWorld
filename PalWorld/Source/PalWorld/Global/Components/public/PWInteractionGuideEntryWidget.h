#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PWInteractionGuideTypes.h"
#include "PWInteractionGuideEntryWidget.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS(Blueprintable)
class PALWORLD_API UPWInteractionGuideEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PW|Interaction")
	void SetGuideAction(const FPWInteractionGuideAction& NewAction);

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	FPWInteractionGuideAction GetGuideAction() const { return GuideAction; }

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Interaction")
	TObjectPtr<UTextBlock> KeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Interaction")
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Interaction")
	TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Interaction")
	TObjectPtr<UProgressBar> HoldProgressBar;

private:
	UPROPERTY(Transient)
	FPWInteractionGuideAction GuideAction;
};
