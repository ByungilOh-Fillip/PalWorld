#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PW_WorldTimeSubsystem.generated.h"

UCLASS()
class PALWORLD_API UPW_WorldTimeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	void ToggleDayNight();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "PW|World Time", meta = (ClampMin = "1.0"))
	float RealSecondsPerGameDay = 1200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "PW|World Time", meta = (ClampMin = "0.1"))
	float TimeUpdateIntervalSeconds = 1.0f;

private:
	FTimerHandle TimeUpdateTimerHandle;
	float CurrentTimeOfDay = 9.0f;
	int32 CurrentDay = 1;

	bool CanRunAuthorityTime() const;
	void UpdateWorldTime();
	void AdvanceTime(float DeltaSeconds);
	void PushTimeToGameState();
};
