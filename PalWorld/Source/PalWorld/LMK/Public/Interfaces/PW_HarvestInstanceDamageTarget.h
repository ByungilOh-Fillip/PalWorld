#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PW_HarvestInstanceDamageTarget.generated.h"

UINTERFACE(BlueprintType)
class PALWORLD_API UPW_HarvestInstanceDamageTarget : public UInterface
{
	GENERATED_BODY()
};

class PALWORLD_API IPW_HarvestInstanceDamageTarget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Harvest")
	bool ApplyHarvestDamageToInstance(int32 InstanceIndex, float DamageAmount, AActor* InstigatorActor);
};
