// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Core/PWPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Player/Core/PWPlayerCharacter.h"

APWPlayerController::APWPlayerController()
{
	bReplicates = true;
}

void APWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (InputSubsystem && GameplayMappingContext)
	{
		InputSubsystem->AddMappingContext(GameplayMappingContext, GameplayMappingPriority);
	}
}

void APWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APWPlayerController::HandleMove);
	}

	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APWPlayerController::HandleLook);
	}

	if (JumpAction)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APWPlayerController::HandleJumpStarted);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APWPlayerController::HandleJumpCompleted);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Canceled, this, &APWPlayerController::HandleJumpCompleted);
	}

	if (SprintAction)
	{
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APWPlayerController::HandleSprintStarted);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APWPlayerController::HandleSprintCompleted);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &APWPlayerController::HandleSprintCompleted);
	}

	if (CrouchAction)
	{
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &APWPlayerController::HandleCrouchStarted);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &APWPlayerController::HandleCrouchCompleted);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Canceled, this, &APWPlayerController::HandleCrouchCompleted);
	}
}

void APWPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->Move(Value.Get<FVector2D>());
	}
}

void APWPlayerController::HandleLook(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->Look(Value.Get<FVector2D>());
	}
}

void APWPlayerController::HandleJumpStarted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StartJump();
	}
}

void APWPlayerController::HandleJumpCompleted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StopJump();
	}
}

void APWPlayerController::HandleSprintStarted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StartSprint();
	}
}

void APWPlayerController::HandleSprintCompleted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StopSprint();
	}
}

void APWPlayerController::HandleCrouchStarted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StartCrouch();
	}
}

void APWPlayerController::HandleCrouchCompleted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StopCrouch();
	}
}

APWPlayerCharacter* APWPlayerController::GetPWPlayerCharacter() const
{
	return Cast<APWPlayerCharacter>(GetPawn());
}
