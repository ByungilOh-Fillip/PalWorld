// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWPlayerCaptureComponent.generated.h"

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerCaptureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerCaptureComponent();
};
