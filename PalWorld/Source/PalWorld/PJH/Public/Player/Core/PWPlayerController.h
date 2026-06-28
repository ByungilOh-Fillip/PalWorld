// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PWPlayerController.generated.h"

class APWPlayerCharacter;
class UInputAction;
class UInputMappingContext;
class UPWPlayerHUDWidget;
struct FInputActionValue;

UCLASS()
class PALWORLD_API APWPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APWPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;
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

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> RollAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Gather")
	TObjectPtr<UInputAction> GatherAction;

	UPROPERTY(EditDefaultsOnly, Category = "Player|UI")
	TSubclassOf<UPWPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UPWPlayerHUDWidget> PlayerHUDWidget;

	void HandleMove(const FInputActionValue& Value);
	void HandleMoveCompleted(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleJumpStarted(const FInputActionValue& Value);
	void HandleJumpCompleted(const FInputActionValue& Value);
	void HandleSprintStarted(const FInputActionValue& Value);
	void HandleSprintCompleted(const FInputActionValue& Value);
	void HandleCrouchStarted(const FInputActionValue& Value);
	void HandleCrouchCompleted(const FInputActionValue& Value);
	void HandleRollStarted(const FInputActionValue& Value);
	void HandleGatherStarted(const FInputActionValue& Value);

	APWPlayerCharacter* GetPWPlayerCharacter() const;
	void CreatePlayerHUD();
	void InitializePlayerHUD();
};
