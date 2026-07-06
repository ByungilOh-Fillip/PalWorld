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

	UPROPERTY(EditDefaultsOnly, Category = "Input|Action")
	TObjectPtr<UInputAction> PrimaryAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Action")
	TObjectPtr<UInputAction> AimAction;

	// 장비 변경은 마우스 휠 전용이다. 숫자키 1/2/3/4는 팰/스피어 조작에 사용한다.
	UPROPERTY(EditDefaultsOnly, Category = "Input|Equipment")
	TObjectPtr<UInputAction> EquipmentWheelNextAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Equipment")
	TObjectPtr<UInputAction> EquipmentWheelPreviousAction;

	UPROPERTY(EditDefaultsOnly, Category = "Player|UI")
	TSubclassOf<UPWPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Debug|Stats")
	bool bEnableDebugStatHotkeys = true;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Debug|Stats", meta = (ClampMin = "0.0"))
	float DebugHealthDamageAmount = 25.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Debug|Stats", meta = (ClampMin = "0.0"))
	float DebugShieldDamageAmount = 25.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Debug|Stats", meta = (ClampMin = "0.0"))
	float DebugHungerConsumeAmount = 25.f;

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
	void HandlePrimaryActionStarted(const FInputActionValue& Value);
	void HandlePrimaryActionCompleted(const FInputActionValue& Value);
	void HandleInteractPressed();
	void HandleAimStarted(const FInputActionValue& Value);
	void HandleAimCompleted(const FInputActionValue& Value);
	void HandleEquipmentWheelNextStarted(const FInputActionValue& Value);
	void HandleEquipmentWheelPreviousStarted(const FInputActionValue& Value);
	void DebugApplyHealthDamage();
	void DebugApplyDirectHealthDamage();
	void DebugConsumeShield();
	void DebugConsumeHunger();

	UFUNCTION(Server, Reliable)
	void ServerDebugApplyHealthDamage();

	UFUNCTION(Server, Reliable)
	void ServerDebugApplyDirectHealthDamage();

	UFUNCTION(Server, Reliable)
	void ServerDebugConsumeShield();

	UFUNCTION(Server, Reliable)
	void ServerDebugConsumeHunger();

	APWPlayerCharacter* GetPWPlayerCharacter() const;
	void CreatePlayerHUD();
	void InitializePlayerHUD();
	void SetCrosshairVisible(bool bVisible);
	void ToggleInventoryMenu();
	void SetInventoryVisible(bool bVisible);
};
