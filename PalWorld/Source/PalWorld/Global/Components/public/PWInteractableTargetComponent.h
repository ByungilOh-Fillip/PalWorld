#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWInteractionGuideTypes.h"
#include "PWInteractableTargetComponent.generated.h"

class UPWInteractionGuideWidget;
class UWidgetComponent;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWInteractableTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWInteractableTargetComponent();

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	bool IsInteractionEnabled() const { return bInteractionEnabled; }

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction")
	void SetInteractionEnabled(bool bNewInteractionEnabled);

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	float GetInteractionRadius() const { return InteractionRadius; }

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction")
	void SetInteractionRadius(float NewInteractionRadius);

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	FText GetPromptText() const { return PromptText; }

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction")
	void SetPromptText(const FText& NewPromptText);

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	int32 GetPriority() const { return Priority; }

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction")
	void SetPriority(int32 NewPriority);

	UFUNCTION(BlueprintPure, Category = "PW|Interaction")
	FVector GetInteractionLocation() const;

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction|Guide")
	void SetInteractionGuideVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction|Guide")
	void SetInteractionGuideActions(const TArray<FPWInteractionGuideAction>& NewGuideActions);

	UFUNCTION(BlueprintPure, Category = "PW|Interaction|Guide")
	void GetInteractionGuideActions(TArray<FPWInteractionGuideAction>& OutActions) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction|Guide")
	void SetInteractionGuideActionProgress(FName ActionId, float NewProgress);

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction|Guide")
	void ClearInteractionGuideActionProgress(FName ActionId);

	UFUNCTION(BlueprintCallable, Category = "PW|Interaction|Guide")
	void RefreshInteractionGuideWidget();

	UFUNCTION(BlueprintPure, Category = "PW|Interaction|Guide")
	UWidgetComponent* GetInteractionGuideWidgetComponent() const { return InteractionGuideWidgetComponent; }

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction", meta = (ClampMin = "0.0"))
	float InteractionRadius = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction")
	int32 Priority = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction")
	FVector InteractionPointOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction")
	bool bInteractionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction|Guide")
	FName DefaultActionId = TEXT("Default");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction|Guide")
	FKey DefaultActionKey = EKeys::F;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction|Guide")
	TArray<FPWInteractionGuideAction> InteractionGuideActions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction|Guide")
	FVector GuideWidgetOffset = FVector(0.0f, 0.0f, 120.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction|Guide")
	FVector2D GuideWidgetDrawSize = FVector2D(360.0f, 180.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Interaction|Guide")
	TSubclassOf<UPWInteractionGuideWidget> InteractionGuideWidgetClass;

private:
	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> InteractionGuideWidgetComponent;

	UPROPERTY(Transient)
	TMap<FName, float> InteractionGuideActionProgressById;

	void EnsureInteractionGuideWidgetComponent();
	void ApplyInteractionGuideWidgetActions(const TArray<FPWInteractionGuideAction>& GuideActions);
};
