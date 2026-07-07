#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PWMultiInteractable.generated.h"

UINTERFACE(BlueprintType)
class PALWORLD_API UPWMultiInteractable : public UInterface
{
	GENERATED_BODY()
};

class PALWORLD_API IPWMultiInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Interaction")
	bool CanInteractAction(AActor* Interactor, FName ActionId) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Interaction")
	bool InteractAction(AActor* Interactor, FName ActionId);
};
