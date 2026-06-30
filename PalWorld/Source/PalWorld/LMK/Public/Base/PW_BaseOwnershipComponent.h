#pragma once

#include "CoreMinimal.h"
#include "Base/PW_BaseTypes.h"
#include "Components/ActorComponent.h"
#include "PW_BaseOwnershipComponent.generated.h"

class APlayerController;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_BaseOwnershipComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_BaseOwnershipComponent();

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Ownership")
	bool CanAccess(const FPW_BaseOwnerId& BaseOwnerId, const FPW_BaseOwnerId& RequesterOwnerId) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Ownership")
	bool CanModify(const FPW_BaseOwnerId& BaseOwnerId, const FPW_BaseOwnerId& RequesterOwnerId) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Ownership")
	bool CanUseStorage(const FPW_BaseOwnerId& BaseOwnerId, const FPW_BaseOwnerId& RequesterOwnerId) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Ownership")
	bool CanAssignPal(const FPW_BaseOwnerId& BaseOwnerId, const FPW_BaseOwnerId& RequesterOwnerId) const;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Ownership")
	FPW_BaseOwnerId MakeOwnerIdFromPlayerController(const APlayerController* PlayerController) const;
};
