#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PW_WorldStateProvider.generated.h"

UINTERFACE(BlueprintType)
class PALWORLD_API UPW_WorldStateProvider : public UInterface
{
	GENERATED_BODY()
};

class PALWORLD_API IPW_WorldStateProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|World State")
	int32 GetCurrentDay() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|World State")
	float GetCurrentTimeOfDay() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|World State")
	bool IsNight() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|World State")
	bool IsDaytime() const;
};
