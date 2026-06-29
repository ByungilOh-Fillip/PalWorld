#include "World/PW_WorldGameState.h"

#include "Net/UnrealNetwork.h"

APW_WorldGameState::APW_WorldGameState()
{
	bReplicates = true;
}

void APW_WorldGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APW_WorldGameState, CurrentDay);
	DOREPLIFETIME(APW_WorldGameState, CurrentTimeOfDay);
}

void APW_WorldGameState::SetWorldTime(int32 NewCurrentDay, float NewCurrentTimeOfDay)
{
	if (!HasAuthority())
	{
		return;
	}

	const bool bPreviousIsNight = IsNightAtTime(CurrentTimeOfDay);

	CurrentDay = FMath::Max(1, NewCurrentDay);
	CurrentTimeOfDay = FMath::Fmod(NewCurrentTimeOfDay, 24.0f);
	if (CurrentTimeOfDay < 0.0f)
	{
		CurrentTimeOfDay += 24.0f;
	}

	BroadcastWorldTimeChanged();
	BroadcastDayNightIfChanged(bPreviousIsNight);
}

float APW_WorldGameState::GetDayStartHour() const
{
	return DayStartHour;
}

float APW_WorldGameState::GetNightStartHour() const
{
	return NightStartHour;
}

int32 APW_WorldGameState::GetCurrentDay_Implementation() const
{
	return CurrentDay;
}

float APW_WorldGameState::GetCurrentTimeOfDay_Implementation() const
{
	return CurrentTimeOfDay;
}

bool APW_WorldGameState::IsNight_Implementation() const
{
	return IsNightAtTime(CurrentTimeOfDay);
}

bool APW_WorldGameState::IsDaytime_Implementation() const
{
	return !IsNightAtTime(CurrentTimeOfDay);
}

void APW_WorldGameState::OnRep_CurrentDay()
{
	BroadcastWorldTimeChanged();
}

void APW_WorldGameState::OnRep_CurrentTimeOfDay(float PreviousTimeOfDay)
{
	const bool bPreviousIsNight = IsNightAtTime(PreviousTimeOfDay);
	BroadcastWorldTimeChanged();
	BroadcastDayNightIfChanged(bPreviousIsNight);
}

bool APW_WorldGameState::IsNightAtTime(float TimeOfDay) const
{
	if (NightStartHour > DayStartHour)
	{
		return TimeOfDay >= NightStartHour || TimeOfDay < DayStartHour;
	}

	return TimeOfDay >= NightStartHour && TimeOfDay < DayStartHour;
}

void APW_WorldGameState::BroadcastWorldTimeChanged()
{
	OnWorldTimeChanged.Broadcast(CurrentDay, CurrentTimeOfDay);
}

void APW_WorldGameState::BroadcastDayNightIfChanged(bool bPreviousIsNight)
{
	const bool bCurrentIsNight = IsNightAtTime(CurrentTimeOfDay);
	if (bCurrentIsNight != bPreviousIsNight)
	{
		OnWorldDayNightChanged.Broadcast(bCurrentIsNight);
	}
}
