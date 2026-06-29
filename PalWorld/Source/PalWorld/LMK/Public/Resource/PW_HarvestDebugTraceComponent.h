#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PW_HarvestDebugTraceComponent.generated.h"

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_HarvestDebugTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_HarvestDebugTraceComponent();

	UFUNCTION(BlueprintCallable, Category = "PW|Harvest|Debug")
	void TryHarvestForward();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Harvest|Debug", meta = (ClampMin = "1.0"))
	float TraceDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Harvest|Debug", meta = (ClampMin = "0.0"))
	float HarvestDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Harvest|Debug")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

private:
	UFUNCTION(Server, Reliable)
	void ServerTryHarvestForward(FVector_NetQuantize TraceStart, FVector_NetQuantizeNormal TraceDirection);

	bool TraceHarvestTarget(FHitResult& OutHit, FVector_NetQuantize& OutTraceStart, FVector_NetQuantizeNormal& OutTraceDirection) const;
	bool TraceHarvestTargetFromView(FHitResult& OutHit, const FVector& TraceStart, const FVector& TraceDirection) const;
	bool ApplyHarvestDamageToHit(const FHitResult& HitResult) const;
	AController* GetOwningController() const;
};
