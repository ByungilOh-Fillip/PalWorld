// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PWPlayerController.generated.h"

class APWPlayerCharacter;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class PALWORLD_API APWPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APWPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> GameplayMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	int32 GameplayMappingPriority = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> CrouchAction;

	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleJumpStarted(const FInputActionValue& Value);
	void HandleJumpCompleted(const FInputActionValue& Value);
	void HandleSprintStarted(const FInputActionValue& Value);
	void HandleSprintCompleted(const FInputActionValue& Value);
	void HandleCrouchStarted(const FInputActionValue& Value);
	void HandleCrouchCompleted(const FInputActionValue& Value);

	APWPlayerCharacter* GetPWPlayerCharacter() const;
};
