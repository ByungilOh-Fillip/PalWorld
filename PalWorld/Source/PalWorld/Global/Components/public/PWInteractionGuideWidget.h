#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PWInteractionGuideTypes.h"
#include "PWInteractionGuideWidget.generated.h"

class UVerticalBox;
class UPWInteractionGuideEntryWidget;

UCLASS(Blueprintable)
class PALWORLD_API UPWInteractionGuideWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PW|Interaction")
	void SetGuideActions(const TArray<FPWInteractionGuideAction>& NewActions);

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	void GetGuideActions(TArray<FPWInteractionGuideAction>& OutActions) const { OutActions = GuideActions; }

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "PW|Interaction")
	TObjectPtr<UVerticalBox> GuideList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Interaction")
	TSubclassOf<UPWInteractionGuideEntryWidget> EntryWidgetClass;

private:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "PW|Interaction", meta = (AllowPrivateAccess = "true"))
	TArray<FPWInteractionGuideAction> GuideActions;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPWInteractionGuideEntryWidget>> EntryWidgets;

	void RebuildGuideList();
};
