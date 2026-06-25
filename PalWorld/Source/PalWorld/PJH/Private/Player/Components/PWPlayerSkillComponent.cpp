// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerSkillComponent.h"

UPWPlayerSkillComponent::UPWPlayerSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}
