// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PWPlayerGameplayTypes.generated.h"

UENUM(BlueprintType)
enum class EPWToolType : uint8
{
	Hand,
	Axe,
	Pickaxe
};

UENUM(BlueprintType)
enum class EPWResourceType : uint8
{
	Tree,
	Stone
};

UENUM(BlueprintType)
enum class EPWResourceDropType : uint8
{
	Wood,
	Stone
};
