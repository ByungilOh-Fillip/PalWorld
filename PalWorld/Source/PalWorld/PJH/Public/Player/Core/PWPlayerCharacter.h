// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PWPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class PALWORLD_API APWPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APWPlayerCharacter();

	void Move(const FVector2D& MovementVector);
	void Look(const FVector2D& LookVector);
	void StartJump();
	void StopJump();
	void StartSprint();
	void StopSprint();
	void StartCrouch();
	void StopCrouch();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Player|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Player|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement")
	float WalkSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Movement")
	float SprintSpeed = 900.f;

	UPROPERTY(ReplicatedUsing = OnRep_IsSprinting)
	bool bIsSprinting = false;

	UFUNCTION()
	void OnRep_IsSprinting();

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewIsSprinting);

	void SetSprinting(bool bNewIsSprinting);
	void ApplyMovementSpeed();
};
