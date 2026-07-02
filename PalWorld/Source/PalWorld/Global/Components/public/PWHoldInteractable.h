#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PWHoldInteractable.generated.h"

UINTERFACE(BlueprintType)
class PALWORLD_API UPWHoldInteractable : public UInterface
{
	GENERATED_BODY()
};

class PALWORLD_API IPWHoldInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Interaction")
	bool CanBeginHoldInteraction(AActor* Interactor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Interaction")
	bool BeginHoldInteraction(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Interaction")
	void EndHoldInteraction(AActor* Interactor);
};
