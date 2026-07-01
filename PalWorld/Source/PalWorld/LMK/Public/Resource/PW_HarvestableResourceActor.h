#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/PW_HarvestDamageTarget.h"
#include "PW_HarvestableResourceActor.generated.h"

class UStaticMeshComponent;
class UPW_HarvestableResourceComponent;

UCLASS()
class PALWORLD_API APW_HarvestableResourceActor : public AActor, public IPW_HarvestDamageTarget
{
	GENERATED_BODY()

public:
	APW_HarvestableResourceActor();

	virtual bool ApplyHarvestDamage_Implementation(float DamageAmount, AActor* InstigatorActor) override;

	UFUNCTION(BlueprintPure, Category = "PW|Harvest")
	UPW_HarvestableResourceComponent* GetHarvestableResourceComponent() const { return ResourceComponent; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Harvest")
	TObjectPtr<UStaticMeshComponent> ResourceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Harvest")
	TObjectPtr<UPW_HarvestableResourceComponent> ResourceComponent;

private:
	UFUNCTION()
	void HandleDepletedStateChanged(bool bNewIsDepleted);

	void ApplyDepletedVisualState(bool bNewIsDepleted);
};
