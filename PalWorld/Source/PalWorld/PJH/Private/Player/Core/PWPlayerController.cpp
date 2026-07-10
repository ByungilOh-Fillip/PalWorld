// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Core/PWPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "Player/Components/PWPlayerStatComponent.h"
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

	if (InputComponent)
	{
		// 메뉴 입력은 지금 단계에서 확실히 동작해야 하므로 IMC와 별도로 직접 바인딩한다.
		InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &APWPlayerController::ToggleInventoryMenu);
		InputComponent->BindKey(EKeys::F, IE_Pressed, this, &APWPlayerController::HandleInteractPressed);
		InputComponent->BindKey(EKeys::F, IE_Released, this, &APWPlayerController::HandleInteractReleased);

		if (bEnableDebugStatHotkeys)
		{
			InputComponent->BindKey(EKeys::NumPadOne, IE_Pressed, this, &APWPlayerController::DebugApplyHealthDamage);
			InputComponent->BindKey(EKeys::NumPadTwo, IE_Pressed, this, &APWPlayerController::DebugApplyDirectHealthDamage);
			InputComponent->BindKey(EKeys::NumPadThree, IE_Pressed, this, &APWPlayerController::DebugConsumeShield);
			InputComponent->BindKey(EKeys::NumPadFour, IE_Pressed, this, &APWPlayerController::DebugConsumeHunger);
		}
	}

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

	if (PrimaryAction)
	{
		EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Started, this, &APWPlayerController::HandlePrimaryActionStarted);
		EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Completed, this, &APWPlayerController::HandlePrimaryActionCompleted);
		EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Canceled, this, &APWPlayerController::HandlePrimaryActionCompleted);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWPrimaryAction] PrimaryAction input asset is not assigned on PlayerController."));
	}

	if (AimAction)
	{
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APWPlayerController::HandleAimStarted);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APWPlayerController::HandleAimCompleted);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Canceled, this, &APWPlayerController::HandleAimCompleted);
	}

	if (CaptureSphereAction)
	{
		EnhancedInputComponent->BindAction(CaptureSphereAction, ETriggerEvent::Started, this, &APWPlayerController::HandleSphereAimStarted);
		EnhancedInputComponent->BindAction(CaptureSphereAction, ETriggerEvent::Completed, this, &APWPlayerController::HandleSphereAimCompleted);
		EnhancedInputComponent->BindAction(CaptureSphereAction, ETriggerEvent::Canceled, this, &APWPlayerController::HandleSphereAimCompleted);
	}

	if (PalSummonAction)
	{
		EnhancedInputComponent->BindAction(PalSummonAction, ETriggerEvent::Started, this, &APWPlayerController::HandlePalSummonPressed);
	}

	if (PalPreviousAction)
	{
		EnhancedInputComponent->BindAction(PalPreviousAction, ETriggerEvent::Started, this, &APWPlayerController::HandlePalPreviousPressed);
	}

	if (PalNextAction)
	{
		EnhancedInputComponent->BindAction(PalNextAction, ETriggerEvent::Started, this, &APWPlayerController::HandlePalNextPressed);
	}

	if (EquipmentWheelNextAction)
	{
		EnhancedInputComponent->BindAction(EquipmentWheelNextAction, ETriggerEvent::Started, this, &APWPlayerController::HandleEquipmentWheelNextStarted);
	}

	if (EquipmentWheelPreviousAction)
	{
		EnhancedInputComponent->BindAction(EquipmentWheelPreviousAction, ETriggerEvent::Started, this, &APWPlayerController::HandleEquipmentWheelPreviousStarted);
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

void APWPlayerController::HandlePrimaryActionStarted(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("[PWPrimaryAction] Primary action input started."));

	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StartPrimaryAction();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWPrimaryAction] Primary action input ignored. PlayerCharacter is missing."));
	}
}

void APWPlayerController::HandlePrimaryActionCompleted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StopPrimaryAction();
	}
}

void APWPlayerController::HandleInteractPressed()
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StartInteract();
	}
}

void APWPlayerController::HandleInteractReleased()
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StopInteract();
	}
}

void APWPlayerController::HandlePalSummonPressed()
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->ToggleSummonPartyPal();
	}
}

void APWPlayerController::HandlePalPreviousPressed()
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->SelectPreviousPartyPal();
	}
}

void APWPlayerController::HandlePalNextPressed()
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->SelectNextPartyPal();
	}
}

void APWPlayerController::HandleAimStarted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		if (PlayerCharacter->StartAim())
		{
			SetCrosshairVisible(true);
		}
	}
}

void APWPlayerController::HandleAimCompleted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->StopAim();
	}

	RefreshCrosshairVisibility();
}

void APWPlayerController::HandleSphereAimStarted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		if (PlayerCharacter->StartSphereAim())
		{
			SetCrosshairVisible(true);
		}
	}
}

void APWPlayerController::HandleSphereAimCompleted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->ReleaseSphereAim();
	}

	RefreshCrosshairVisibility();
}

void APWPlayerController::HandleEquipmentWheelNextStarted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->SelectNextEquipmentSlot();
	}
}

void APWPlayerController::HandleEquipmentWheelPreviousStarted(const FInputActionValue& Value)
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		PlayerCharacter->SelectPreviousEquipmentSlot();
	}
}

void APWPlayerController::DebugApplyHealthDamage()
{
	if (bEnableDebugStatHotkeys)
	{
		ServerDebugApplyHealthDamage();
	}
}

void APWPlayerController::DebugApplyDirectHealthDamage()
{
	if (bEnableDebugStatHotkeys)
	{
		ServerDebugApplyDirectHealthDamage();
	}
}

void APWPlayerController::DebugConsumeShield()
{
	if (bEnableDebugStatHotkeys)
	{
		ServerDebugConsumeShield();
	}
}

void APWPlayerController::DebugConsumeHunger()
{
	if (bEnableDebugStatHotkeys)
	{
		ServerDebugConsumeHunger();
	}
}

void APWPlayerController::ServerDebugApplyHealthDamage_Implementation()
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		if (UPWPlayerStatComponent* StatComponent = PlayerCharacter->GetStatComponent())
		{
			StatComponent->ApplyHealthDamage(DebugHealthDamageAmount);
			UE_LOG(LogTemp, Display, TEXT("[PWDebugStats] Apply damage. Amount=%.1f Health=%.1f/%.1f Shield=%.1f/%.1f"),
				DebugHealthDamageAmount,
				StatComponent->GetCurrentHealth(),
				StatComponent->GetMaxHealth(),
				StatComponent->GetCurrentShield(),
				StatComponent->GetMaxShield());
		}
	}
}

void APWPlayerController::ServerDebugApplyDirectHealthDamage_Implementation()
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		if (UPWPlayerStatComponent* StatComponent = PlayerCharacter->GetStatComponent())
		{
			StatComponent->ApplyDirectHealthDamage(DebugHealthDamageAmount);
			UE_LOG(LogTemp, Display, TEXT("[PWDebugStats] Apply direct health damage. Amount=%.1f Health=%.1f/%.1f"),
				DebugHealthDamageAmount,
				StatComponent->GetCurrentHealth(),
				StatComponent->GetMaxHealth());
		}
	}
}

void APWPlayerController::ServerDebugConsumeShield_Implementation()
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		if (UPWPlayerStatComponent* StatComponent = PlayerCharacter->GetStatComponent())
		{
			StatComponent->ConsumeShield(DebugShieldDamageAmount);
			UE_LOG(LogTemp, Display, TEXT("[PWDebugStats] Consume shield. Amount=%.1f Shield=%.1f/%.1f"),
				DebugShieldDamageAmount,
				StatComponent->GetCurrentShield(),
				StatComponent->GetMaxShield());
		}
	}
}

void APWPlayerController::ServerDebugConsumeHunger_Implementation()
{
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
		if (UPWPlayerStatComponent* StatComponent = PlayerCharacter->GetStatComponent())
		{
			StatComponent->ConsumeHunger(DebugHungerConsumeAmount);
			UE_LOG(LogTemp, Display, TEXT("[PWDebugStats] Consume hunger. Amount=%.1f Hunger=%.1f/%.1f"),
				DebugHungerConsumeAmount,
				StatComponent->GetCurrentHunger(),
				StatComponent->GetMaxHunger());
		}
	}
}

APWPlayerCharacter* APWPlayerController::GetPWPlayerCharacter() const
{
	return Cast<APWPlayerCharacter>(GetPawn());
}

void APWPlayerController::CreatePlayerHUD()
{
	if (!IsLocalController() || PlayerHUDWidget)
	{
		return;
	}

	if (!PlayerHUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWInventory] PlayerHUDWidgetClass is not assigned on %s."), *GetName());
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

void APWPlayerController::SetCrosshairVisible(bool bVisible)
{
	if (!IsLocalController())
	{
		return;
	}

	CreatePlayerHUD();

	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->SetCrosshairVisible(bVisible);
	}
}

void APWPlayerController::RefreshCrosshairVisibility()
{
	const APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter();
	SetCrosshairVisible(PlayerCharacter && (PlayerCharacter->IsAiming() || PlayerCharacter->IsSphereAiming()));
}

void APWPlayerController::ToggleInventoryMenu()
{
	CreatePlayerHUD();

	const bool bNewInventoryVisible = PlayerHUDWidget ? !PlayerHUDWidget->IsInventoryVisible() : true;
	SetInventoryVisible(bNewInventoryVisible);
}

void APWPlayerController::SetInventoryVisible(bool bVisible)
{
	if (!IsLocalController())
	{
		return;
	}

	CreatePlayerHUD();

	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->SetInventoryVisible(bVisible);
	}

	bShowMouseCursor = bVisible;

	if (bVisible)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		return;
	}

	SetInputMode(FInputModeGameOnly());
}
