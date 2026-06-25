// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerInteractionComponent.h"

UPWPlayerInteractionComponent::UPWPlayerInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}
