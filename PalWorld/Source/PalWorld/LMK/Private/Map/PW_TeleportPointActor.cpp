#include "Map/PW_TeleportPointActor.h"

#include "Components/SceneComponent.h"
#include "Net/UnrealNetwork.h"

APW_TeleportPointActor::APW_TeleportPointActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void APW_TeleportPointActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APW_TeleportPointActor, TeleportPointId);
	DOREPLIFETIME(APW_TeleportPointActor, bDiscovered);
	DOREPLIFETIME(APW_TeleportPointActor, bCanTeleport);
}

void APW_TeleportPointActor::SetDiscovered(bool bNewDiscovered)
{
	if (HasAuthority())
	{
		bDiscovered = bNewDiscovered;
		ForceNetUpdate();
	}
}
