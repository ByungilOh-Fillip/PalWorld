// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerCaptureComponent.h"

UPWPlayerCaptureComponent::UPWPlayerCaptureComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}
