// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/PWWorldItemDropLibrary.h"

#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Player/Data/PWItemDataAsset.h"
#include "World/PWWorldItemActor.h"

namespace
{
FVector ResolveBaseLocation(const FPWWorldItemDropRequest& Request)
{
	if (!Request.SourceLocation.IsNearlyZero())
	{
		return Request.SourceLocation + Request.LocationOffset;
	}

	return Request.SourceActor
		? Request.SourceActor->GetActorLocation() + Request.LocationOffset
		: Request.LocationOffset;
}

bool ResolveTargetLocation(const FPWWorldItemDropRequest& Request, FVector& OutTargetLocation)
{
	if (Request.TargetActor)
	{
		OutTargetLocation = Request.TargetActor->GetActorLocation();
		return true;
	}

	if (!Request.TargetLocation.IsNearlyZero())
	{
		OutTargetLocation = Request.TargetLocation;
		return true;
	}

	return false;
}

FVector MakeDropImpulse(const FPWWorldItemDropRequest& Request, const FVector& SourceLocation, const FVector& TargetLocation, bool bHasTargetLocation)
{
	FVector Direction = bHasTargetLocation ? TargetLocation - SourceLocation : FVector::ZeroVector;
	Direction.Z = 0.f;

	if (Direction.IsNearlyZero() && Request.SourceActor)
	{
		Direction = Request.SourceActor->GetActorForwardVector();
		Direction.Z = 0.f;
	}

	if (Direction.IsNearlyZero())
	{
		Direction = FVector::ForwardVector;
	}

	const float HorizontalImpulse = FMath::FRandRange(
		FMath::Max(0.f, Request.MinHorizontalImpulse),
		FMath::Max(Request.MinHorizontalImpulse, Request.MaxHorizontalImpulse));
	const float UpwardImpulse = FMath::FRandRange(
		FMath::Max(0.f, Request.MinUpwardImpulse),
		FMath::Max(Request.MinUpwardImpulse, Request.MaxUpwardImpulse));

	return Direction.GetSafeNormal() * HorizontalImpulse + FVector(0.f, 0.f, UpwardImpulse);
}

float MakeAutoCollectDelay(const FPWWorldItemDropRequest& Request)
{
	const float MinDelay = FMath::Max(0.f, Request.AutoCollectMinDelay);
	const float MaxDelay = FMath::Max(MinDelay, Request.AutoCollectMaxDelay);
	return MaxDelay > 0.f ? FMath::FRandRange(MinDelay, MaxDelay) : 0.f;
}
}

APWWorldItemActor* UPWWorldItemDropLibrary::SpawnWorldItemDrop(UObject* WorldContextObject, const FPWWorldItemDropRequest& Request)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World || Request.Count <= 0 || !CanSpawnWorldItem(Request.ItemData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWWorldDrop] Spawn rejected. World=%s ItemData=%s Count=%d"),
			World ? TEXT("valid") : TEXT("none"),
			*GetNameSafe(Request.ItemData),
			Request.Count);
		return nullptr;
	}

	if (Request.SourceActor && !Request.SourceActor->HasAuthority())
	{
		return nullptr;
	}

	TSubclassOf<APWWorldItemActor> ItemActorClass = Request.WorldItemActorClass;
	if (!ItemActorClass)
	{
		ItemActorClass = APWWorldItemActor::StaticClass();
	}

	const FVector SourceLocation = ResolveBaseLocation(Request);
	FVector TargetLocation = FVector::ZeroVector;
	const bool bHasTargetLocation = ResolveTargetLocation(Request, TargetLocation);

	FVector SpawnLocation = SourceLocation;
	if (bHasTargetLocation)
	{
		const float MinAlpha = FMath::Clamp(Request.TowardTargetMinAlpha, 0.f, 1.f);
		const float MaxAlpha = FMath::Clamp(FMath::Max(MinAlpha, Request.TowardTargetMaxAlpha), 0.f, 1.f);
		const float TowardTargetAlpha = FMath::FRandRange(MinAlpha, MaxAlpha);
		SpawnLocation.X = FMath::Lerp(SourceLocation.X, TargetLocation.X, TowardTargetAlpha);
		SpawnLocation.Y = FMath::Lerp(SourceLocation.Y, TargetLocation.Y, TowardTargetAlpha);
	}

	const float ScatterRadius = FMath::Max(0.f, Request.ScatterRadius);
	SpawnLocation += FVector(
		FMath::FRandRange(-ScatterRadius, ScatterRadius),
		FMath::FRandRange(-ScatterRadius, ScatterRadius),
		0.f);

	TArray<AActor*> GroundTraceIgnoredActors;
	if (Request.bIgnoreSourceActorInGroundTrace && Request.SourceActor)
	{
		GroundTraceIgnoredActors.Add(Request.SourceActor);
	}
	if (Request.TargetActor)
	{
		GroundTraceIgnoredActors.Add(Request.TargetActor);
	}

	SpawnLocation = APWWorldItemActor::ResolveGroundedDropLocation(World, SpawnLocation, GroundTraceIgnoredActors);
	const FRotator SpawnRotation(0.f, FMath::FRandRange(0.f, 360.f), 0.f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Request.SourceActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APWWorldItemActor* WorldItem = World->SpawnActor<APWWorldItemActor>(ItemActorClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!WorldItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWWorldDrop] Spawn actor failed. Item=%s x%d Class=%s"),
			*Request.ItemData->GetItemId().ToString(),
			Request.Count,
			*GetNameSafe(ItemActorClass.Get()));
		return nullptr;
	}

	const FVector DropImpulse = MakeDropImpulse(Request, SourceLocation, TargetLocation, bHasTargetLocation);
	WorldItem->InitializeWorldItem(Request.ItemData, Request.ItemId, Request.Count, Request.PreferredReceiver, DropImpulse);

	if (Request.bStartAutoCollect)
	{
		WorldItem->StartAutoCollect(MakeAutoCollectDelay(Request), Request.AutoCollectRadius);
	}

	UE_LOG(LogTemp, Verbose, TEXT("[PWWorldDrop] Spawned world item. Item=%s x%d Location=%s Receiver=%s AutoCollect=%s"),
		*Request.ItemData->GetItemId().ToString(),
		Request.Count,
		*SpawnLocation.ToString(),
		*GetNameSafe(Request.PreferredReceiver.Get()),
		Request.bStartAutoCollect ? TEXT("true") : TEXT("false"));

	return WorldItem;
}

bool UPWWorldItemDropLibrary::CanSpawnWorldItem(UPWItemDataAsset* ItemData)
{
	return ItemData && (ItemData->GetWorldMesh() || ItemData->GetEquipmentStaticMesh());
}
