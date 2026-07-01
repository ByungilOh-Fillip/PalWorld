#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PWInteractable.generated.h"

UINTERFACE(BlueprintType)
class PALWORLD_API UPWInteractable : public UInterface
{
	GENERATED_BODY()
};

class PALWORLD_API IPWInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Interaction")
	bool CanInteract(AActor* Interactor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Interaction")
	bool Interact(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Interaction")
	FText GetInteractionPrompt() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Interaction")
	int32 GetInteractionPriority() const;
};
