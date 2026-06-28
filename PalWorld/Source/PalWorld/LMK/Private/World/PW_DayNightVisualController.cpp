#include "World/PW_DayNightVisualController.h"

#include "Components/SceneComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "EngineUtils.h"
#include "Components/LightComponent.h"
#include "World/PW_WorldGameState.h"

APW_DayNightVisualController::APW_DayNightVisualController()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void APW_DayNightVisualController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bHasTargetSunRotation)
	{
		return;
	}

	if (SunLight != nullptr)
	{
		const FRotator InterpolatedSunRotation = FMath::RInterpTo(
			SunLight->GetActorRotation(),
			TargetSunRotation,
			DeltaTime,
			RotationInterpolationSpeed);
		SunLight->SetActorRotation(InterpolatedSunRotation);

		if (ULightComponent* SunLightComponent = SunLight->GetLightComponent())
		{
			SunLightComponent->SetIntensity(FMath::FInterpTo(
				SunLightComponent->Intensity,
				TargetSunIntensity,
				DeltaTime,
				LightInterpolationSpeed));
			SunLightComponent->SetLightColor(FLinearColor::LerpUsingHSV(
				SunLightComponent->GetLightColor(),
				TargetSunColor,
				FMath::Clamp(DeltaTime * LightInterpolationSpeed, 0.0f, 1.0f)));
		}
	}

	if (MoonLight != nullptr)
	{
		const FRotator InterpolatedMoonRotation = FMath::RInterpTo(
			MoonLight->GetActorRotation(),
			TargetMoonRotation,
			DeltaTime,
			RotationInterpolationSpeed);
		MoonLight->SetActorRotation(InterpolatedMoonRotation);

		if (ULightComponent* MoonLightComponent = MoonLight->GetLightComponent())
		{
			MoonLightComponent->SetIntensity(FMath::FInterpTo(
				MoonLightComponent->Intensity,
				TargetMoonIntensity,
				DeltaTime,
				LightInterpolationSpeed));
			MoonLightComponent->SetLightColor(FLinearColor::LerpUsingHSV(
				MoonLightComponent->GetLightColor(),
				TargetMoonColor,
				FMath::Clamp(DeltaTime * LightInterpolationSpeed, 0.0f, 1.0f)));
		}
	}

	if (SkyLight != nullptr)
	{
		if (USkyLightComponent* SkyLightComponent = SkyLight->GetLightComponent())
		{
			CurrentSkyLightIntensity = FMath::FInterpTo(
				CurrentSkyLightIntensity,
				TargetSkyLightIntensity,
				DeltaTime,
				LightInterpolationSpeed);
			CurrentSkyLightColor = FLinearColor::LerpUsingHSV(
				CurrentSkyLightColor,
				TargetSkyLightColor,
				FMath::Clamp(DeltaTime * LightInterpolationSpeed, 0.0f, 1.0f));

			SkyLightComponent->SetIntensity(CurrentSkyLightIntensity);
			SkyLightComponent->SetLightColor(CurrentSkyLightColor);
		}
	}
}

void APW_DayNightVisualController::BeginPlay()
{
	Super::BeginPlay();

	ResolveSunLight();
	ResolveSkyLight();
	BindWorldGameState();
}

void APW_DayNightVisualController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindWorldGameState();

	Super::EndPlay(EndPlayReason);
}

void APW_DayNightVisualController::HandleWorldTimeChanged(int32 CurrentDay, float CurrentTimeOfDay)
{
	SetTargetLighting(CurrentTimeOfDay);
}

void APW_DayNightVisualController::BindWorldGameState()
{
	UWorld* World = GetWorld();
	CachedWorldGameState = World != nullptr ? World->GetGameState<APW_WorldGameState>() : nullptr;
	if (CachedWorldGameState == nullptr)
	{
		return;
	}

	CachedWorldGameState->OnWorldTimeChanged.AddDynamic(this, &APW_DayNightVisualController::HandleWorldTimeChanged);
	SetTargetLighting(IPW_WorldStateProvider::Execute_GetCurrentTimeOfDay(CachedWorldGameState));
}

void APW_DayNightVisualController::UnbindWorldGameState()
{
	if (CachedWorldGameState != nullptr)
	{
		CachedWorldGameState->OnWorldTimeChanged.RemoveDynamic(this, &APW_DayNightVisualController::HandleWorldTimeChanged);
	}
}

void APW_DayNightVisualController::ResolveSunLight()
{
	if (SunLight != nullptr || !bAutoFindSunLight)
	{
		return;
	}

	for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
	{
		SunLight = *It;
		return;
	}
}

void APW_DayNightVisualController::ResolveSkyLight()
{
	if (SkyLight != nullptr || !bAutoFindSkyLight)
	{
		return;
	}

	for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
	{
		SkyLight = *It;
		return;
	}
}

void APW_DayNightVisualController::SetTargetLighting(float CurrentTimeOfDay)
{
	if (SunLight == nullptr && MoonLight == nullptr && SkyLight == nullptr)
	{
		return;
	}

	const float SunPitch = CalculateSunPitch(CurrentTimeOfDay);
	const float MoonPitch = CalculateMoonPitch(CurrentTimeOfDay);
	const float DayAlpha = CalculateDayAlpha(CurrentTimeOfDay);

	TargetSunRotation = FRotator(SunPitch, SunYaw, 0.0f);
	TargetMoonRotation = FRotator(MoonPitch, MoonYaw, 0.0f);
	TargetSunIntensity = FMath::Lerp(NightSunIntensity, DaySunIntensity, DayAlpha);
	TargetMoonIntensity = FMath::Lerp(NightMoonIntensity, DayMoonIntensity, DayAlpha);
	TargetSkyLightIntensity = FMath::Lerp(NightSkyLightIntensity, DaySkyLightIntensity, DayAlpha);
	TargetSunColor = FLinearColor::LerpUsingHSV(NightSunColor, DaySunColor, DayAlpha);
	TargetMoonColor = FLinearColor::LerpUsingHSV(NightMoonColor, DayMoonColor, DayAlpha);
	TargetSkyLightColor = FLinearColor::LerpUsingHSV(NightSkyLightColor, DaySkyLightColor, DayAlpha);
	bHasTargetSunRotation = true;

	if (bSnapOnFirstUpdate && !bHasAppliedInitialSunRotation)
	{
		ApplyLightingInstantly();
		bHasAppliedInitialSunRotation = true;
	}
}

void APW_DayNightVisualController::ApplyLightingInstantly()
{
	if (SunLight != nullptr)
	{
		SunLight->SetActorRotation(TargetSunRotation);
		if (ULightComponent* SunLightComponent = SunLight->GetLightComponent())
		{
			SunLightComponent->SetIntensity(TargetSunIntensity);
			SunLightComponent->SetLightColor(TargetSunColor);
		}
	}

	if (MoonLight != nullptr)
	{
		MoonLight->SetActorRotation(TargetMoonRotation);
		if (ULightComponent* MoonLightComponent = MoonLight->GetLightComponent())
		{
			MoonLightComponent->SetIntensity(TargetMoonIntensity);
			MoonLightComponent->SetLightColor(TargetMoonColor);
		}
	}

	if (SkyLight != nullptr)
	{
		if (USkyLightComponent* SkyLightComponent = SkyLight->GetLightComponent())
		{
			CurrentSkyLightIntensity = TargetSkyLightIntensity;
			CurrentSkyLightColor = TargetSkyLightColor;
			SkyLightComponent->SetIntensity(TargetSkyLightIntensity);
			SkyLightComponent->SetLightColor(TargetSkyLightColor);
		}
	}
}

float APW_DayNightVisualController::CalculateSunPitch(float CurrentTimeOfDay) const
{
	const float NormalizedTime = FMath::Fmod(CurrentTimeOfDay, 24.0f);

	if (NormalizedTime < 6.0f)
	{
		return FMath::Lerp(MidnightPitch, SunrisePitch, NormalizedTime / 6.0f);
	}

	if (NormalizedTime < 12.0f)
	{
		return FMath::Lerp(SunrisePitch, NoonPitch, (NormalizedTime - 6.0f) / 6.0f);
	}

	if (NormalizedTime < 18.0f)
	{
		return FMath::Lerp(NoonPitch, SunsetPitch, (NormalizedTime - 12.0f) / 6.0f);
	}

	return FMath::Lerp(SunsetPitch, MidnightPitch, (NormalizedTime - 18.0f) / 6.0f);
}

float APW_DayNightVisualController::CalculateMoonPitch(float CurrentTimeOfDay) const
{
	const float NormalizedTime = FMath::Fmod(CurrentTimeOfDay, 24.0f);

	if (NormalizedTime < 6.0f)
	{
		return FMath::Lerp(MoonMidnightPitch, MoonHorizonPitch, NormalizedTime / 6.0f);
	}

	if (NormalizedTime >= 18.0f)
	{
		return FMath::Lerp(MoonHorizonPitch, MoonMidnightPitch, (NormalizedTime - 18.0f) / 6.0f);
	}

	return MidnightPitch;
}

float APW_DayNightVisualController::CalculateDayAlpha(float CurrentTimeOfDay) const
{
	const float NormalizedTime = FMath::Fmod(CurrentTimeOfDay, 24.0f);

	if (NormalizedTime < 5.0f)
	{
		return 0.0f;
	}

	if (NormalizedTime < 7.0f)
	{
		return (NormalizedTime - 5.0f) / 2.0f;
	}

	if (NormalizedTime < 17.0f)
	{
		return 1.0f;
	}

	if (NormalizedTime < 19.0f)
	{
		return 1.0f - ((NormalizedTime - 17.0f) / 2.0f);
	}

	return 0.0f;
}
