// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "PWPlayerCaptureComponent.generated.h"

class APWPlayerCharacter;
class APWCaptureSphereProjectile;
class APWPalBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPWCaptureAimInfoChanged);

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerCaptureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerCaptureComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool HasThrowableCaptureSphere() const;
	bool TryThrowCaptureSphere(const FVector& ThrowDirection);

	UFUNCTION(BlueprintPure, Category = "Player|Capture|Aim")
	bool IsCaptureAimVisible() const { return bCaptureAimVisible; }

	UFUNCTION(BlueprintPure, Category = "Player|Capture|Aim")
	bool HasCaptureAimTarget() const { return bHasCaptureAimTarget; }

	UFUNCTION(BlueprintPure, Category = "Player|Capture|Aim")
	float GetCaptureAimChance() const { return CaptureAimChance; }

	UFUNCTION(BlueprintPure, Category = "Player|Capture|Aim")
	int32 GetCaptureAimChancePercent() const { return FMath::RoundToInt(CaptureAimChance * 100.f); }

	UFUNCTION(BlueprintPure, Category = "Player|Capture|Aim")
	FText GetCaptureAimTargetNameText() const;

	UFUNCTION(BlueprintPure, Category = "Player|Capture")
	int32 GetThrowableCaptureSphereCount() const;

	UPROPERTY(BlueprintAssignable, Category = "Player|Capture|Aim")
	FPWCaptureAimInfoChanged OnCaptureAimInfoChanged;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture")
	FName CaptureSphereItemId = TEXT("PalSphere");

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture", meta = (ClampMin = "1"))
	int32 CaptureSphereCost = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Projectile")
	TSubclassOf<APWCaptureSphereProjectile> CaptureSphereProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Projectile", meta = (ClampMin = "0.0"))
	float ProjectileSpawnForwardOffset = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Projectile")
	float ProjectileSpawnUpOffset = 55.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Aim", meta = (ClampMin = "0.0"))
	float CaptureAimTraceDistance = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Aim", meta = (ClampMin = "0.0"))
	float CaptureAimTraceRadius = 36.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Aim", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseCaptureChance = 0.18f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Aim", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MissingHealthCaptureChanceBonus = 0.72f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Aim", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinimumCaptureChance = 0.02f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Aim", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaximumCaptureChance = 0.95f;

	UPROPERTY(Transient)
	TWeakObjectPtr<APWPalBase> CaptureAimTarget;

	UPROPERTY(Transient)
	bool bCaptureAimVisible = false;

	UPROPERTY(Transient)
	bool bHasCaptureAimTarget = false;

	UPROPERTY(Transient)
	float CaptureAimChance = 0.f;

	APWPlayerCharacter* GetOwnerCharacter() const;
	void UpdateCaptureAimInfo();
	void ClearCaptureAimInfo();
	void SetCaptureAimInfo(bool bNewVisible, APWPalBase* NewTarget, float NewChance);
	float CalculateCaptureChance(APWPalBase* TargetPal) const;
	void ThrowCaptureSphere_Server(const FVector& ThrowDirection);

	UFUNCTION(Server, Reliable)
	void ServerThrowCaptureSphere(FVector_NetQuantizeNormal ThrowDirection);
};
