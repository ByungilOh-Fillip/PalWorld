#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWInteractableTargetComponent.generated.h"

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
};
