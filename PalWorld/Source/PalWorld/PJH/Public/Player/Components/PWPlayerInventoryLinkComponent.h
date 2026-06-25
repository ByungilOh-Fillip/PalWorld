// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWPlayerInventoryLinkComponent.generated.h"

UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPWPlayerInventoryLinkComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWPlayerInventoryLinkComponent();
};
