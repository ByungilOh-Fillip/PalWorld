// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPalCommandComponent.h"

UPWPalCommandComponent::UPWPalCommandComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}
