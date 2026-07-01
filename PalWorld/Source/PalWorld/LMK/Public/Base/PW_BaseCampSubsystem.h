#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BaseTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "PW_BaseCampSubsystem.generated.h"

class APW_BaseCampActor;

UCLASS()
class PALWORLD_API UPW_BaseCampSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterBaseCamp(APW_BaseCampActor* BaseCamp);
	void UnregisterBaseCamp(APW_BaseCampActor* BaseCamp);

	UFUNCTION(BlueprintCallable, Category = "PW|Base")
	APW_BaseCampActor* FindBaseCampAtLocation(const FVector& Location) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base")
	void GetBaseCampsForOwner(const FPW_BaseOwnerId& OwnerId, TArray<APW_BaseCampActor*>& OutBaseCamps) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base")
	bool CanPlaceBaseCampAtLocation(const FVector& Location, float Radius, APW_BaseCampActor* IgnoredBaseCamp = nullptr) const;

private:
	UPROPERTY()
	TArray<TObjectPtr<APW_BaseCampActor>> RegisteredBaseCamps;

	void CleanupInvalidBaseCamps();
};
