#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PW_HarvestDamageTarget.generated.h"

UINTERFACE(BlueprintType)
class PALWORLD_API UPW_HarvestDamageTarget : public UInterface
{
	GENERATED_BODY()
};

class PALWORLD_API IPW_HarvestDamageTarget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Harvest")
	bool ApplyHarvestDamage(float DamageAmount, AActor* InstigatorActor);
};
