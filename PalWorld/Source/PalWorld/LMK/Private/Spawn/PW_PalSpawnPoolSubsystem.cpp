#include "Spawn/PW_PalSpawnPoolSubsystem.h"

#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Interfaces/PW_PooledSpawnActor.h"
#include "PWPalBase.h"
#include "Spawn/PW_WildPalSpawner.h"

bool UPW_PalSpawnPoolSubsystem::CanRunAuthorityPool() const
{
	const UWorld* World = GetWorld();
	return World != nullptr && World->GetAuthGameMode() != nullptr;
}

APWPalBase* UPW_PalSpawnPoolSubsystem::AcquirePal(TSubclassOf<APWPalBase> PalClass, const FTransform& SpawnTransform, APW_WildPalSpawner* OwningSpawner)
{
	if (!CanRunAuthorityPool() || PalClass == nullptr)
	{
		return nullptr;
	}

	TArray<TObjectPtr<APWPalBase>>& ClassPool = PooledActorsByClass.FindOrAdd(PalClass).Actors;
	while (!ClassPool.IsEmpty())
	{
		APWPalBase* ReusedPal = ClassPool.Pop();
		if (IsValid(ReusedPal))
		{
			ActivateActor(ReusedPal, SpawnTransform, OwningSpawner);
			return ReusedPal;
		}
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwningSpawner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APWPalBase* SpawnedPal = World->SpawnActor<APWPalBase>(PalClass, SpawnTransform, SpawnParams);
	if (SpawnedPal == nullptr)
	{
		return nullptr;
	}

	if (!SpawnedPal->GetIsReplicated())
	{
		UE_LOG(LogTemp, Warning, TEXT("PW_PalSpawnPoolSubsystem: Spawned Pal '%s' is not replicated."), *SpawnedPal->GetName());
	}

	ActivateActor(SpawnedPal, SpawnTransform, OwningSpawner);
	return SpawnedPal;
}

void UPW_PalSpawnPoolSubsystem::ReturnPal(APWPalBase* PalActor, APW_WildPalSpawner* OwningSpawner)
{
	if (!CanRunAuthorityPool() || !IsValid(PalActor))
	{
		return;
	}

	DeactivateActor(PalActor, OwningSpawner);
	PooledActorsByClass.FindOrAdd(PalActor->GetClass()).Actors.Add(PalActor);
	TrimPoolIfNeeded();
}

int32 UPW_PalSpawnPoolSubsystem::GetPooledActorCount() const
{
	int32 Count = 0;
	for (const TPair<TSubclassOf<APWPalBase>, FPW_PooledPalActorList>& PoolPair : PooledActorsByClass)
	{
		Count += PoolPair.Value.Actors.Num();
	}
	return Count;
}

void UPW_PalSpawnPoolSubsystem::ActivateActor(APWPalBase* PalActor, const FTransform& SpawnTransform, APW_WildPalSpawner* OwningSpawner) const
{
	PalActor->SetActorTransform(SpawnTransform);
	PalActor->SetOwner(OwningSpawner);
	PalActor->SetActorHiddenInGame(false);
	PalActor->SetActorEnableCollision(true);
	PalActor->SetActorTickEnabled(true);
	TArray<UActorComponent*> Components;
	PalActor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component != nullptr)
		{
			Component->SetComponentTickEnabled(true);
		}
	}
	PalActor->FlushNetDormancy();
	PalActor->SetNetDormancy(DORM_Awake);
	PalActor->ForceNetUpdate();

	if (PalActor->GetClass()->ImplementsInterface(UPW_PooledSpawnActor::StaticClass()))
	{
		IPW_PooledSpawnActor::Execute_OnActivatedFromPool(PalActor, OwningSpawner);
	}
}

void UPW_PalSpawnPoolSubsystem::DeactivateActor(APWPalBase* PalActor, APW_WildPalSpawner* OwningSpawner) const
{
	if (PalActor->GetClass()->ImplementsInterface(UPW_PooledSpawnActor::StaticClass()))
	{
		IPW_PooledSpawnActor::Execute_OnReturnedToPool(PalActor, OwningSpawner);
	}

	// TODO: Stop/restart Pal AI once the Pal AI component/controller contract exists.
	PalActor->SetActorHiddenInGame(true);
	PalActor->SetActorEnableCollision(false);
	PalActor->SetActorTickEnabled(false);
	TArray<UActorComponent*> Components;
	PalActor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component != nullptr)
		{
			Component->SetComponentTickEnabled(false);
		}
	}
	PalActor->ForceNetUpdate();
	PalActor->SetNetDormancy(DORM_DormantAll);
	PalActor->SetOwner(nullptr);
}

void UPW_PalSpawnPoolSubsystem::TrimPoolIfNeeded()
{
	int32 PooledCount = GetPooledActorCount();
	if (PooledCount <= MaxPooledActors)
	{
		return;
	}

	for (TPair<TSubclassOf<APWPalBase>, FPW_PooledPalActorList>& PoolPair : PooledActorsByClass)
	{
		TArray<TObjectPtr<APWPalBase>>& ClassPool = PoolPair.Value.Actors;
		while (PooledCount > MaxPooledActors && !ClassPool.IsEmpty())
		{
			APWPalBase* PalToDestroy = ClassPool.Pop();
			if (IsValid(PalToDestroy))
			{
				PalToDestroy->Destroy();
			}
			--PooledCount;
		}

		if (PooledCount <= MaxPooledActors)
		{
			break;
		}
	}
}
