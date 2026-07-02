#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BaseTypes.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PW_BaseWorkTargetRegistryComponent.generated.h"

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_BaseWorkTargetRegistryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_BaseWorkTargetRegistryComponent();

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Work")
	void RegisterWorkTarget(AActor* TargetActor, FName WorkTargetId, FGameplayTag RequiredWorkTag);

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Work")
	void UnregisterWorkTarget(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Work")
	bool FindWorkTargetByTag(FGameplayTag RequiredWorkTag, FPW_WorkTargetEntry& OutEntry) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Work")
	void GetWorkTargetsByTag(FGameplayTag RequiredWorkTag, TArray<FPW_WorkTargetEntry>& OutEntries) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Work")
	void GetAllWorkTargets(TArray<FPW_WorkTargetEntry>& OutEntries) const;

private:
	UPROPERTY()
	TArray<FPW_WorkTargetEntry> WorkTargets;
};
