#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PW_TemperatureVolume.generated.h"

class UBoxComponent;

UCLASS()
class PALWORLD_API APW_TemperatureVolume : public AActor
{
	GENERATED_BODY()

public:
	APW_TemperatureVolume();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Temperature")
	TObjectPtr<UBoxComponent> TemperatureBounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Temperature")
	float BaseTemperature = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Temperature")
	float NightTemperatureModifier = -10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Temperature")
	float ColdThreshold = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Temperature")
	float HeatThreshold = 40.0f;

private:
	UFUNCTION()
	void OnTemperatureBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnTemperatureEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

	float CalculateEffectiveTemperature() const;
	void ApplyTemperatureToActor(AActor* TargetActor) const;
	void ClearTemperatureFromActor(AActor* TargetActor) const;
};
