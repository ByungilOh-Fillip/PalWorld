#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PW_ItemReceiver.generated.h"

UINTERFACE(BlueprintType)
class PALWORLD_API UPW_ItemReceiver : public UInterface
{
	GENERATED_BODY()
};

class PALWORLD_API IPW_ItemReceiver
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PW|Item")
	bool ReceiveItem(FName ItemId, int32 Count);
};
