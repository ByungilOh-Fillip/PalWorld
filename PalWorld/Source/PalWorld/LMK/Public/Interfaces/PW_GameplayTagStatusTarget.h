#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "PW_GameplayTagStatusTarget.generated.h"

UINTERFACE(BlueprintType)
class PALWORLD_API UPW_GameplayTagStatusTarget : public UInterface
{
	GENERATED_BODY()
};

class PALWORLD_API IPW_GameplayTagStatusTarget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Status")
	void ApplyStatusTag(FGameplayTag StatusTag);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Status")
	void RemoveStatusTag(FGameplayTag StatusTag);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Status")
	bool HasStatusTag(FGameplayTag StatusTag) const;
};
