// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerCombatComponent.h"

UPWPlayerCombatComponent::UPWPlayerCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}
