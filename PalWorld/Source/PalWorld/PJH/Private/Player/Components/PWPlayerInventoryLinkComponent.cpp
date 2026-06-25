// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerInventoryLinkComponent.h"

UPWPlayerInventoryLinkComponent::UPWPlayerInventoryLinkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}
