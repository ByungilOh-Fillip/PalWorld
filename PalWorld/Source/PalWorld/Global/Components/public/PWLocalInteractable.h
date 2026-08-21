#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PWLocalInteractable.generated.h"

UINTERFACE(BlueprintType)
class PALWORLD_API UPWLocalInteractable : public UInterface
{
	GENERATED_BODY()
};

class PALWORLD_API IPWLocalInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Interaction")
	bool CanLocalInteract(AActor* Interactor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Interaction")
	bool LocalInteract(AActor* Interactor);
};
