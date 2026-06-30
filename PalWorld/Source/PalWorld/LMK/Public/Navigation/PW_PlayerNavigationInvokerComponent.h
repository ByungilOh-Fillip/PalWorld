#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PW_PlayerNavigationInvokerComponent.generated.h"

class UNavigationInvokerComponent;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_PlayerNavigationInvokerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_PlayerNavigationInvokerComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "PW|Navigation")
	void ConfigureNavigationInvoker();

	UFUNCTION(BlueprintCallable, Category = "PW|Navigation")
	void SetPlayerNavigationInvokerActive(bool bActive);

	UFUNCTION(BlueprintPure, Category = "PW|Navigation")
	bool IsPlayerNavigationInvokerReady() const;

	UFUNCTION(BlueprintPure, Category = "PW|Navigation")
	float GetResolvedGenerationRadius() const;

	UFUNCTION(BlueprintPure, Category = "PW|Navigation")
	static float GetRecommendedGenerationRadiusForWildPalSpawner(float ActivationDistance, float SpawnRadius);

	UFUNCTION(BlueprintCallable, Category = "PW|Navigation")
	void ApplyWildPalSpawnerCompatibleDefaults();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Navigation", meta = (ClampMin = "0.0"))
	float GenerationRadius = 11000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Navigation", meta = (ClampMin = "0.0"))
	float RemovalRadius = 13000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Navigation")
	bool bAutoConfigureOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Navigation")
	bool bAutoActivateNavigationInvoker = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Navigation")
	bool bUseOwnerNetCullDistanceAsFallback = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Navigation", meta = (ClampMin = "0.0"))
	float MinGenerationRadius = 8000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Navigation", meta = (ClampMin = "0.0"))
	float MaxGenerationRadius = 15000.0f;

private:
	UNavigationInvokerComponent* FindNavigationInvokerComponent() const;
};
