// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Data/PWPrimaryActionDataAsset.h"

UPWPrimaryActionDataAsset::UPWPrimaryActionDataAsset()
{
	FPWPrimaryActionToolDamage HandDamage;
	HandDamage.ToolType = EPWToolType::Hand;
	HandDamage.TreeDamage = 5.f;
	HandDamage.StoneDamage = 3.f;

	FPWPrimaryActionToolDamage AxeDamage;
	AxeDamage.ToolType = EPWToolType::Axe;
	AxeDamage.TreeDamage = 25.f;
	AxeDamage.StoneDamage = 6.f;

	FPWPrimaryActionToolDamage PickaxeDamage;
	PickaxeDamage.ToolType = EPWToolType::Pickaxe;
	PickaxeDamage.TreeDamage = 8.f;
	PickaxeDamage.StoneDamage = 25.f;

	ToolDamages = { HandDamage, AxeDamage, PickaxeDamage };
}

float UPWPrimaryActionDataAsset::GetDamage(EPWToolType ToolType, EPWResourceType ResourceType) const
{
	const FPWPrimaryActionToolDamage* ToolDamage = ToolDamages.FindByPredicate(
		[ToolType](const FPWPrimaryActionToolDamage& Candidate)
		{
			return Candidate.ToolType == ToolType;
		});

	if (!ToolDamage)
	{
		return 0.f;
	}

	return ResourceType == EPWResourceType::Stone
		? ToolDamage->StoneDamage
		: ToolDamage->TreeDamage;
}

UAnimMontage* UPWPrimaryActionDataAsset::GetPrimaryActionMontage(EPWToolType ToolType) const
{
	return ToolType == EPWToolType::Hand ? HandMontage : HarvestMontage;
}

float UPWPrimaryActionDataAsset::GetActionDurationForTool(EPWToolType ToolType) const
{
	if (ToolType == EPWToolType::Hand)
	{
		return HandActionDuration;
	}

	if (ToolType == EPWToolType::Axe || ToolType == EPWToolType::Pickaxe)
	{
		return HarvestActionDuration;
	}

	return ActionDuration;
}

float UPWPrimaryActionDataAsset::GetAnimationPlayRate(EPWToolType ToolType) const
{
	return ToolType == EPWToolType::Hand ? HandAnimationPlayRate : HarvestAnimationPlayRate;
}

float UPWPrimaryActionDataAsset::GetFallbackHitTime(EPWToolType ToolType) const
{
	return ToolType == EPWToolType::Hand ? HandFallbackHitTime : HarvestFallbackHitTime;
}
