// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/Actor.h"
#include "PWCaptureSphereProjectile.generated.h"

class APWPalBase;
class UMaterialInterface;
class UProjectileMovementComponent;
class UPrimitiveComponent;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UStaticMesh;

UENUM(BlueprintType)
enum class EPWCaptureSphereProjectileState : uint8
{
	Flying,
	HitReact,
	Opening,
	Closing,
	Capturing,
	Finished
};

UCLASS()
class PALWORLD_API APWCaptureSphereProjectile : public AActor
{
	GENERATED_BODY()

public:
	APWCaptureSphereProjectile();

	void LaunchInDirection(const FVector& Direction);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Player|Capture")
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(VisibleAnywhere, Category = "Player|Capture|Visual")
	TObjectPtr<USceneComponent> VisualRoot;

	UPROPERTY(VisibleAnywhere, Category = "Player|Capture|Visual")
	TObjectPtr<UStaticMeshComponent> SphereVisual;

	UPROPERTY(VisibleAnywhere, Category = "Player|Capture|Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Movement", meta = (ClampMin = "0.0"))
	float LifeSeconds = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Movement", meta = (ClampMin = "0"))
	int32 MaxWorldBounceCount = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Movement", meta = (ClampMin = "0.0"))
	float StopBounceSpeed = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Visual", meta = (ClampMin = "0.0"))
	float FlyingSpinSpeed = 720.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Visual")
	TObjectPtr<UStaticMesh> ClosedSphereMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Visual")
	TObjectPtr<UStaticMesh> OpenSphereMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Visual")
	TObjectPtr<UMaterialInterface> BodyMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Visual")
	TObjectPtr<UMaterialInterface> GlowMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Sequence", meta = (ClampMin = "0.0"))
	float HitReactDuration = 0.22f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Sequence", meta = (ClampMin = "0.0"))
	float HitReactBackDistance = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Sequence", meta = (ClampMin = "0.0"))
	float HitReactUpHeight = 180.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Sequence", meta = (ClampMin = "0.0"))
	float OpenDuration = 0.22f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Sequence", meta = (ClampMin = "0.0"))
	float CloseDuration = 0.18f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Sequence", meta = (ClampMin = "0.0"))
	float CaptureShakeDuration = 2.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Sequence", meta = (ClampMin = "0.0"))
	float CaptureShakeYawAmplitude = 18.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Capture|Sequence", meta = (ClampMin = "0.0"))
	float CaptureShakeFrequency = 8.f;

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileState)
	EPWCaptureSphereProjectileState ProjectileState = EPWCaptureSphereProjectileState::Flying;

	UPROPERTY(Replicated)
	float StateElapsedTime = 0.f;

	UPROPERTY()
	TWeakObjectPtr<AActor> HitTarget;

	UPROPERTY(ReplicatedUsing = OnRep_HitReactLocations)
	FVector_NetQuantize HitReactStartLocation = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_HitReactLocations)
	FVector_NetQuantize HitReactTargetLocation = FVector::ZeroVector;

	int32 WorldBounceCount = 0;

	UFUNCTION()
	void HandleProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void HandleProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	UFUNCTION()
	void OnRep_ProjectileState();

	UFUNCTION()
	void OnRep_HitReactLocations();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartHitReact(FVector_NetQuantize StartLocation, FVector_NetQuantize TargetLocation);

	void EnterState(EPWCaptureSphereProjectileState NewState);
	void UpdateCaptureSequence(float DeltaSeconds);
	void UpdateFlyingVisual(float DeltaSeconds);
	void StartHitReact(const FHitResult& Hit, AActor* CaptureTarget);
	void UpdateHitReact(float DeltaSeconds);
	void ApplyStateVisual();
	void ApplyCleanMaterials();
	bool IsCaptureTarget(AActor* OtherActor) const;
	bool HasValidHitReactLocations() const;
	void StopProjectileMovement();
};
