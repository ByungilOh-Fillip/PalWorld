#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PW_BaseNavigationComponent.generated.h"

class UNavigationInvokerComponent;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_BaseNavigationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_BaseNavigationComponent();

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Navigation")
	void ConfigureNavigationInvoker(float CampRadius);

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Navigation")
	void SetBaseNavigationActive(bool bNewActive);

	UFUNCTION(BlueprintPure, Category = "PW|Base|Navigation")
	bool IsBaseNavigationActive() const { return bIsBaseNavigationActive; }

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Navigation")
	bool ProjectPointToBaseNavigation(const FVector& Location, FVector& OutProjectedLocation) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Navigation")
	bool FindPathToLocation(const FVector& StartLocation, const FVector& EndLocation) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Navigation")
	bool GetRandomReachablePointInBase(const FVector& Origin, float Radius, FVector& OutLocation) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Navigation")
	bool IsNavigationReadyForBase() const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Navigation")
	void DrawDebugNavigationSample() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Navigation", meta = (ClampMin = "0.0"))
	float NavRemovalExtraRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Navigation")
	FVector NavigationProjectionExtent = FVector(500.0f, 500.0f, 500.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Navigation|Debug")
	bool bDrawDebugNavigation = true;

private:
	bool bIsBaseNavigationActive = true;

	UNavigationInvokerComponent* FindNavigationInvokerComponent() const;
	float GetBaseCampRadiusFallback() const;
};
