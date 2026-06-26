// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerMountComponent.h"

UPWPlayerMountComponent::UPWPlayerMountComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}
