#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PW_DayNightVisualController.generated.h"

class ADirectionalLight;
class APW_WorldGameState;
class ASkyLight;
class USceneComponent;

UCLASS()
class PALWORLD_API APW_DayNightVisualController : public AActor
{
	GENERATED_BODY()

public:
	APW_DayNightVisualController();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "루트"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "태양 라이트"))
	TObjectPtr<ADirectionalLight> SunLight;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "달 라이트"))
	TObjectPtr<ADirectionalLight> MoonLight;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "하늘 보조광"))
	TObjectPtr<ASkyLight> SkyLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "태양 라이트 자동 탐색"))
	bool bAutoFindSunLight = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "하늘 보조광 자동 탐색"))
	bool bAutoFindSkyLight = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "일출 각도"))
	float SunrisePitch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "정오 각도"))
	float NoonPitch = -90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "일몰 각도"))
	float SunsetPitch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "자정 각도"))
	float MidnightPitch = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "태양 방향"))
	float SunYaw = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "달 방향"))
	float MoonYaw = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "Moon Horizon Pitch"))
	float MoonHorizonPitch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "Moon Midnight Pitch"))
	float MoonMidnightPitch = -35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (ClampMin = "0.0", DisplayName = "낮 태양 밝기"))
	float DaySunIntensity = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (ClampMin = "0.0", DisplayName = "밤 태양 밝기"))
	float NightSunIntensity = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (ClampMin = "0.0", DisplayName = "낮 달 밝기"))
	float DayMoonIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (ClampMin = "0.0", DisplayName = "밤 달 밝기"))
	float NightMoonIntensity = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (ClampMin = "0.0", DisplayName = "낮 하늘 보조광 밝기"))
	float DaySkyLightIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (ClampMin = "0.0", DisplayName = "밤 하늘 보조광 밝기"))
	float NightSkyLightIntensity = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "낮 태양 색"))
	FLinearColor DaySunColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "밤 태양 색"))
	FLinearColor NightSunColor = FLinearColor(0.35f, 0.45f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "낮 달 색"))
	FLinearColor DayMoonColor = FLinearColor(0.3f, 0.35f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "밤 달 색"))
	FLinearColor NightMoonColor = FLinearColor(0.55f, 0.65f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "낮 하늘 보조광 색"))
	FLinearColor DaySkyLightColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "밤 하늘 보조광 색"))
	FLinearColor NightSkyLightColor = FLinearColor(0.08f, 0.12f, 0.25f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (ClampMin = "0.1", DisplayName = "회전 보간 속도"))
	float RotationInterpolationSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (ClampMin = "0.1", DisplayName = "밝기 보간 속도"))
	float LightInterpolationSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Day Night", meta = (DisplayName = "첫 갱신 즉시 적용"))
	bool bSnapOnFirstUpdate = true;

private:
	UPROPERTY()
	TObjectPtr<APW_WorldGameState> CachedWorldGameState;

	FRotator TargetSunRotation = FRotator::ZeroRotator;
	FRotator TargetMoonRotation = FRotator::ZeroRotator;
	float TargetSunIntensity = 0.0f;
	float TargetMoonIntensity = 0.0f;
	float TargetSkyLightIntensity = 0.0f;
	float CurrentSkyLightIntensity = 0.0f;
	FLinearColor TargetSunColor = FLinearColor::White;
	FLinearColor TargetMoonColor = FLinearColor::White;
	FLinearColor TargetSkyLightColor = FLinearColor::White;
	FLinearColor CurrentSkyLightColor = FLinearColor::White;
	bool bHasTargetSunRotation = false;
	bool bHasAppliedInitialSunRotation = false;

	UFUNCTION()
	void HandleWorldTimeChanged(int32 CurrentDay, float CurrentTimeOfDay);

	void BindWorldGameState();
	void UnbindWorldGameState();
	void ResolveSunLight();
	void ResolveSkyLight();
	void SetTargetLighting(float CurrentTimeOfDay);
	void ApplyLightingInstantly();
	float CalculateSunPitch(float CurrentTimeOfDay) const;
	float CalculateMoonPitch(float CurrentTimeOfDay) const;
	float CalculateDayAlpha(float CurrentTimeOfDay) const;
};
