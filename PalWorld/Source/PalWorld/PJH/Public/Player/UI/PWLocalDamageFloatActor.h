// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PWLocalDamageFloatActor.generated.h"

class UTextRenderComponent;

UCLASS(Blueprintable)
class PALWORLD_API APWLocalDamageFloatActor : public AActor
{
	GENERATED_BODY()

public:
	APWLocalDamageFloatActor();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Player|UI|Damage")
	void InitializeDamageFloat(float InDamage);

private:
	UPROPERTY(VisibleAnywhere, Category = "Player|UI|Damage")
	TObjectPtr<UTextRenderComponent> DamageText;

	UPROPERTY(EditDefaultsOnly, Category = "Player|UI|Damage", meta = (ClampMin = "0.05"))
	float LifeTime = 0.85f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|UI|Damage", meta = (ClampMin = "0.0"))
	float FloatSpeed = 70.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|UI|Damage", meta = (ClampMin = "0.0"))
	float HorizontalDrift = 12.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|UI|Damage", meta = (ClampMin = "0.0"))
	float TextWorldSize = 18.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|UI|Damage")
	FLinearColor TextColor = FLinearColor(1.f, 0.92f, 0.35f, 1.f);

	float ElapsedTime = 0.f;
	FVector DriftDirection = FVector::ZeroVector;

	void UpdateBillboardRotation();
	void UpdateTextAlpha() const;
};
