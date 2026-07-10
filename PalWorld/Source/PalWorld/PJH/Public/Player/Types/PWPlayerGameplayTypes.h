// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PWPlayerGameplayTypes.generated.h"

UENUM(BlueprintType)
enum class EPWToolType : uint8
{
	Hand,
	Axe,
	Pickaxe,
	Crossbow
};

UENUM(BlueprintType)
enum class EPWItemType : uint8
{
	Misc,
	Resource,
	Tool,
	Weapon,
	Armor,
	Shield,
	Glider,
	Accessory,
	Consumable,
	Sphere
};

UENUM(BlueprintType)
enum class EPWEquipmentSlotType : uint8
{
	None,
	Weapon,
	Head,
	Body,
	Shield,
	Glider,
	SphereModule,
	Accessory,
	Food
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
