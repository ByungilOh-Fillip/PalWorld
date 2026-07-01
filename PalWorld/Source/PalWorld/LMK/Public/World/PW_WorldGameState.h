#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/PW_WorldStateProvider.h"
#include "PW_WorldGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPW_OnWorldTimeChanged, int32, CurrentDay, float, CurrentTimeOfDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPW_OnWorldDayNightChanged, bool, bIsNight);

UCLASS()
class PALWORLD_API APW_WorldGameState : public AGameStateBase, public IPW_WorldStateProvider
{
	GENERATED_BODY()

public:
	APW_WorldGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "PW|World State")
	void SetWorldTime(int32 NewCurrentDay, float NewCurrentTimeOfDay);

	UFUNCTION(BlueprintCallable, Category = "PW|World State")
	float GetDayStartHour() const;

	UFUNCTION(BlueprintCallable, Category = "PW|World State")
	float GetNightStartHour() const;

	virtual int32 GetCurrentDay_Implementation() const override;
	virtual float GetCurrentTimeOfDay_Implementation() const override;
	virtual bool IsNight_Implementation() const override;
	virtual bool IsDaytime_Implementation() const override;

	UPROPERTY(BlueprintAssignable, Category = "PW|World State")
	FPW_OnWorldTimeChanged OnWorldTimeChanged;

	UPROPERTY(BlueprintAssignable, Category = "PW|World State")
	FPW_OnWorldDayNightChanged OnWorldDayNightChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|World State", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float DayStartHour = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|World State", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float NightStartHour = 18.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentDay, BlueprintReadOnly, Category = "PW|World State")
	int32 CurrentDay = 1;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentTimeOfDay, BlueprintReadOnly, Category = "PW|World State")
	float CurrentTimeOfDay = 6.0f;

	UFUNCTION()
	void OnRep_CurrentDay();

	UFUNCTION()
	void OnRep_CurrentTimeOfDay(float PreviousTimeOfDay);

private:
	bool IsNightAtTime(float TimeOfDay) const;
	void BroadcastWorldTimeChanged();
	void BroadcastDayNightIfChanged(bool bPreviousIsNight);
};
