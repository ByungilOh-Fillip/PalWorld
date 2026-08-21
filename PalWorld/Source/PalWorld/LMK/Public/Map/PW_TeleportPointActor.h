#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PWInteractable.h"
#include "PW_TeleportPointActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UPWInteractableTargetComponent;

UCLASS(Blueprintable)
class PALWORLD_API APW_TeleportPointActor : public AActor, public IPWInteractable
{
	GENERATED_BODY()

public:
	APW_TeleportPointActor();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual bool Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual int32 GetInteractionPriority_Implementation() const override;

	UFUNCTION(BlueprintPure, Category = "PW|Map|Teleport")
	FName GetTeleportPointId() const { return TeleportPointId; }

	UFUNCTION(BlueprintPure, Category = "PW|Map|Teleport")
	FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintPure, Category = "PW|Map|Teleport")
	bool IsDiscovered() const { return bDiscovered; }

	UFUNCTION(BlueprintCallable, Category = "PW|Map|Teleport")
	void SetDiscovered(bool bNewDiscovered);

	UFUNCTION(BlueprintPure, Category = "PW|Map|Teleport")
	bool CanTeleport() const { return bCanTeleport; }

	UFUNCTION(BlueprintPure, Category = "PW|Map|Teleport")
	FVector GetTeleportArrivalLocation() const;

	bool CanUseAsTeleportSource(AActor* Interactor) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport|Interaction")
	TObjectPtr<UPWInteractableTargetComponent> InteractableTargetComponent;

	UPROPERTY(Replicated, EditInstanceOnly, BlueprintReadOnly, Category = "PW|Map|Teleport")
	FName TeleportPointId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport")
	FText DisplayName;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport")
	bool bDiscovered = false;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport")
	bool bCanTeleport = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport", meta = (ClampMin = "0.0"))
	float ArrivalForwardOffset = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport")
	float ArrivalUpOffset = 120.0f;
};
