// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Core/PWPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "Player/UI/PWPlayerHUDWidget.h"

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

	CreatePlayerHUD();
	InitializePlayerHUD();
}

void APWPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	InitializePlayerHUD();
}

void APWPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	InitializePlayerHUD();
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
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APWPlayerController::HandleMoveCompleted);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &APWPlayerController::HandleMoveCompleted);
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

	if (RollAction)
	{
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &APWPlayerController::HandleRollStarted);
	}

	if (GatherAction)
	{
		EnhancedInputComponent->BindAction(GatherAction, ETriggerEvent::Started, this, &APWPlayerController::HandleGatherStarted);
	}
}

// 입력 핸들러는 얇게 유지하고, 권한 판단은 캐릭터/컴포넌트에서 처리한다.
void APWPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->Move(Value.Get<FVector2D>());
	}
}

void APWPlayerController::HandleMoveCompleted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->Move(FVector2D::ZeroVector);
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

void APWPlayerController::HandleRollStarted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StartRoll();
	}
}

void APWPlayerController::HandleGatherStarted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StartGather();
	}
}

APWPlayerCharacter* APWPlayerController::GetPWPlayerCharacter() const
{
	return Cast<APWPlayerCharacter>(GetPawn());
}

void APWPlayerController::CreatePlayerHUD()
{
	if (!IsLocalController() || PlayerHUDWidget || !PlayerHUDWidgetClass)
	{
		return;
	}

	PlayerHUDWidget = CreateWidget<UPWPlayerHUDWidget>(this, PlayerHUDWidgetClass);
	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->AddToViewport();
	}
}

void APWPlayerController::InitializePlayerHUD()
{
	if (!IsLocalController())
	{
		return;
	}

	CreatePlayerHUD();

	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->InitializeWithPlayerCharacter(GetPWPlayerCharacter());
	}
}
