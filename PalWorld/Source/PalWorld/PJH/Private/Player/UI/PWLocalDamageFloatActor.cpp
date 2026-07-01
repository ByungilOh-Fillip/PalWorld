// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWLocalDamageFloatActor.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

APWLocalDamageFloatActor::APWLocalDamageFloatActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;

	DamageText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DamageText"));
	SetRootComponent(DamageText);

	DamageText->SetHorizontalAlignment(EHTA_Center);
	DamageText->SetVerticalAlignment(EVRTA_TextCenter);
	DamageText->SetWorldSize(TextWorldSize);
	DamageText->SetTextRenderColor(TextColor.ToFColor(true));
	DamageText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APWLocalDamageFloatActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedTime += DeltaSeconds;
	if (ElapsedTime >= LifeTime)
	{
		Destroy();
		return;
	}

	const FVector MoveDelta = FVector::UpVector * FloatSpeed * DeltaSeconds + DriftDirection * HorizontalDrift * DeltaSeconds;
	AddActorWorldOffset(MoveDelta);
	UpdateBillboardRotation();
	UpdateTextAlpha();
}

void APWLocalDamageFloatActor::InitializeDamageFloat(float InDamage)
{
	const FString DamageTextString = InDamage < 10.f
		? FString::Printf(TEXT("%.1f"), InDamage)
		: FString::Printf(TEXT("%.0f"), InDamage);

	DamageText->SetText(FText::FromString(DamageTextString));
	DamageText->SetWorldSize(TextWorldSize);
	DamageText->SetTextRenderColor(TextColor.ToFColor(true));

	ElapsedTime = 0.f;
	DriftDirection = FMath::VRand();
	DriftDirection.Z = 0.f;
	DriftDirection.Normalize();

	UpdateBillboardRotation();
}

void APWLocalDamageFloatActor::UpdateBillboardRotation()
{
	const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!CameraManager)
	{
		return;
	}

	const FVector DirectionToCamera = CameraManager->GetCameraLocation() - GetActorLocation();
	if (!DirectionToCamera.IsNearlyZero())
	{
		SetActorRotation(DirectionToCamera.Rotation());
	}
}

void APWLocalDamageFloatActor::UpdateTextAlpha() const
{
	if (!DamageText || LifeTime <= 0.f)
	{
		return;
	}

	const float Alpha = FMath::Clamp(1.f - (ElapsedTime / LifeTime), 0.f, 1.f);
	FColor DrawColor = TextColor.ToFColor(true);
	DrawColor.A = static_cast<uint8>(FMath::RoundToInt(Alpha * 255.f));
	DamageText->SetTextRenderColor(DrawColor);
}
