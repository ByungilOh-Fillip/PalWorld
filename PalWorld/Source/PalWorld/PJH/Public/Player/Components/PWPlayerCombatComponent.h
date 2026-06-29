// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWPlayerCombatComponent.generated.h"

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerCombatComponent();
};
