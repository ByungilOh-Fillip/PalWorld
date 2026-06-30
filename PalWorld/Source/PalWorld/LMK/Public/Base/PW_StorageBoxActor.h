#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PW_StorageBoxActor.generated.h"

class APW_BaseCampActor;
class UStaticMeshComponent;
class UPW_InventoryComponent;

UCLASS(Blueprintable)
class PALWORLD_API APW_StorageBoxActor : public AActor
{
	GENERATED_BODY()

public:
	APW_StorageBoxActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "PW|Storage")
	UPW_InventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "PW|Storage")
	APW_BaseCampActor* GetOwningBaseCamp() const { return OwningBaseCamp; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Storage")
	TObjectPtr<UStaticMeshComponent> StorageMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Storage")
	TObjectPtr<UPW_InventoryComponent> InventoryComponent;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "PW|Storage")
	TObjectPtr<APW_BaseCampActor> OwningBaseCamp;

private:
	void RegisterWithBaseCamp();
	void UnregisterFromBaseCamp();
};
