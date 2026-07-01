#include "World/PW_WorldTimeSubsystem.h"

#include "Engine/World.h"
#include "World/PW_WorldGameState.h"

namespace
{
static void PW_ToggleDayNightConsoleCommand(const TArray<FString>& Args, UWorld* World)
{
	if (World == nullptr || World->GetAuthGameMode() == nullptr)
	{
		return;
	}

	UPW_WorldTimeSubsystem* WorldTimeSubsystem = World->GetSubsystem<UPW_WorldTimeSubsystem>();
	if (WorldTimeSubsystem != nullptr)
	{
		WorldTimeSubsystem->ToggleDayNight();
	}
}

FAutoConsoleCommandWithWorldAndArgs GToggleDayNightCommand(
	TEXT("pw.Time.ToggleDayNight"),
	TEXT("Toggles the authoritative world time between day start and night start."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&PW_ToggleDayNightConsoleCommand));
}

void UPW_WorldTimeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!CanRunAuthorityTime())
	{
		return;
	}

	InWorld.GetTimerManager().SetTimer(
		TimeUpdateTimerHandle,
		this,
		&UPW_WorldTimeSubsystem::UpdateWorldTime,
		TimeUpdateIntervalSeconds,
		true);
	PushTimeToGameState();
}

void UPW_WorldTimeSubsystem::Deinitialize()
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->GetTimerManager().ClearTimer(TimeUpdateTimerHandle);
	}

	Super::Deinitialize();
}

void UPW_WorldTimeSubsystem::ToggleDayNight()
{
	UWorld* World = GetWorld();
	APW_WorldGameState* WorldGameState = World != nullptr ? World->GetGameState<APW_WorldGameState>() : nullptr;
	if (WorldGameState == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PW_WorldTimeSubsystem: Cannot toggle day/night because PW_WorldGameState is not active."));
		return;
	}

	const bool bIsNight = IPW_WorldStateProvider::Execute_IsNight(WorldGameState);
	CurrentTimeOfDay = bIsNight ? 12.0f : 0.0f;
	PushTimeToGameState();

	UE_LOG(LogTemp, Display, TEXT("PW_WorldTimeSubsystem: Toggled day/night. CurrentTimeOfDay=%.2f"), CurrentTimeOfDay);
}

bool UPW_WorldTimeSubsystem::CanRunAuthorityTime() const
{
	const UWorld* World = GetWorld();
	return World != nullptr && World->GetAuthGameMode() != nullptr;
}

void UPW_WorldTimeSubsystem::UpdateWorldTime()
{
	if (!CanRunAuthorityTime())
	{
		return;
	}

	AdvanceTime(TimeUpdateIntervalSeconds);
	PushTimeToGameState();
}

void UPW_WorldTimeSubsystem::AdvanceTime(float DeltaSeconds)
{
	const float HoursPerRealSecond = 24.0f / FMath::Max(RealSecondsPerGameDay, 1.0f);
	CurrentTimeOfDay += DeltaSeconds * HoursPerRealSecond;

	while (CurrentTimeOfDay >= 24.0f)
	{
		CurrentTimeOfDay -= 24.0f;
		++CurrentDay;
	}
}

void UPW_WorldTimeSubsystem::PushTimeToGameState()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	APW_WorldGameState* WorldGameState = World->GetGameState<APW_WorldGameState>();
	if (WorldGameState == nullptr)
	{
		UE_LOG(LogTemp, Verbose, TEXT("PW_WorldTimeSubsystem: PW_WorldGameState is not active."));
		return;
	}

	WorldGameState->SetWorldTime(CurrentDay, CurrentTimeOfDay);
}
