#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PW_TeleportPointActor.generated.h"

class USceneComponent;

UCLASS(Blueprintable)
class PALWORLD_API APW_TeleportPointActor : public AActor
{
	GENERATED_BODY()

public:
	APW_TeleportPointActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(Replicated, EditInstanceOnly, BlueprintReadOnly, Category = "PW|Map|Teleport")
	FName TeleportPointId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport")
	FText DisplayName;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport")
	bool bDiscovered = false;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "PW|Map|Teleport")
	bool bCanTeleport = true;
};
