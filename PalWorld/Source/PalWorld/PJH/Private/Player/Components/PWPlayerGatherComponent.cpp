// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerGatherComponent.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPlayerActionComponent.h"
#include "Player/Components/PWPlayerStatComponent.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "TimerManager.h"
#include "World/Resources/PWGatherableResourceActor.h"

UPWPlayerGatherComponent::UPWPlayerGatherComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPWPlayerGatherComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPWPlayerGatherComponent, CurrentToolType);
}

void UPWPlayerGatherComponent::TryGatherFromView()
{
	APWGatherableResourceActor* TargetResource = FindGatherTargetFromView();
	if (!TargetResource)
	{
		BP_OnGatherFailed();
		return;
	}

	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerRequestGather(TargetResource);
		return;
	}

	GatherAuthority(TargetResource);
}

void UPWPlayerGatherComponent::RequestEquipTool(EPWToolType NewToolType)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerRequestEquipTool(NewToolType);
		return;
	}

	CurrentToolType = NewToolType;
}

void UPWPlayerGatherComponent::ServerRequestGather_Implementation(APWGatherableResourceActor* TargetResource)
{
	GatherAuthority(TargetResource);
}

void UPWPlayerGatherComponent::ServerRequestEquipTool_Implementation(EPWToolType NewToolType)
{
	CurrentToolType = NewToolType;
}

void UPWPlayerGatherComponent::OnRep_CurrentToolType()
{
}

APWPlayerCharacter* UPWPlayerGatherComponent::GetPlayerCharacter() const
{
	return Cast<APWPlayerCharacter>(GetOwner());
}

APWGatherableResourceActor* UPWPlayerGatherComponent::FindGatherTargetFromView() const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		return nullptr;
	}

	const AController* Controller = PlayerCharacter->GetController();
	if (!Controller)
	{
		return nullptr;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * GatherTraceDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWGatherTrace), false, PlayerCharacter);
	QueryParams.AddIgnoredActor(PlayerCharacter);

	FHitResult HitResult;
	const UWorld* World = GetWorld();
	if (!World || !World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return nullptr;
	}

	return Cast<APWGatherableResourceActor>(HitResult.GetActor());
}

bool UPWPlayerGatherComponent::CanGatherTarget(const APWGatherableResourceActor* TargetResource) const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !TargetResource || TargetResource->IsDepleted())
	{
		return false;
	}

	const UPWPlayerActionComponent* ActionComponent = PlayerCharacter->GetActionComponent();
	if (ActionComponent && !ActionComponent->CanStartAction(EPWPlayerActionState::Gathering))
	{
		return false;
	}

	const float DistanceSquared = FVector::DistSquared(PlayerCharacter->GetActorLocation(), TargetResource->GetActorLocation());
	return DistanceSquared <= FMath::Square(GatherTraceDistance + 50.f);
}

float UPWPlayerGatherComponent::ResolveGatherDamage(EPWResourceType ResourceType) const
{
	switch (ResourceType)
	{
	case EPWResourceType::Tree:
		switch (CurrentToolType)
		{
		case EPWToolType::Axe:
			return AxeTreeDamage;
		case EPWToolType::Pickaxe:
			return PickaxeTreeDamage;
		case EPWToolType::Hand:
		default:
			return HandTreeDamage;
		}

	case EPWResourceType::Stone:
		switch (CurrentToolType)
		{
		case EPWToolType::Axe:
			return AxeStoneDamage;
		case EPWToolType::Pickaxe:
			return PickaxeStoneDamage;
		case EPWToolType::Hand:
		default:
			return HandStoneDamage;
		}

	default:
		return 0.f;
	}
}

void UPWPlayerGatherComponent::GatherAuthority(APWGatherableResourceActor* TargetResource)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || !CanGatherTarget(TargetResource))
	{
		BP_OnGatherFailed();
		return;
	}

	UPWPlayerActionComponent* ActionComponent = PlayerCharacter->GetActionComponent();
	if (ActionComponent && !ActionComponent->TryStartActionAuthority(EPWPlayerActionState::Gathering))
	{
		BP_OnGatherFailed();
		return;
	}

	UPWPlayerStatComponent* StatComponent = PlayerCharacter->GetStatComponent();
	if (StatComponent && !StatComponent->TryConsumeStamina(GatherStaminaCost))
	{
		if (ActionComponent)
		{
			ActionComponent->FinishActionAuthority(EPWPlayerActionState::Gathering);
		}
		BP_OnGatherFailed();
		return;
	}

	const float GatherDamage = ResolveGatherDamage(TargetResource->GetResourceType());
	if (!TargetResource->ApplyGatherDamage(PlayerCharacter, GatherDamage, CurrentToolType))
	{
		if (ActionComponent)
		{
			ActionComponent->FinishActionAuthority(EPWPlayerActionState::Gathering);
		}
		BP_OnGatherFailed();
		return;
	}

	BP_OnGatherStarted(TargetResource, CurrentToolType);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GatherActionTimerHandle);
		World->GetTimerManager().SetTimer(
			GatherActionTimerHandle,
			this,
			&UPWPlayerGatherComponent::FinishGatherAction,
			GatherActionDuration,
			false);
	}

	UE_LOG(LogTemp, Log, TEXT("[PWGather] Gather hit. Player=%s Resource=%s Tool=%s Damage=%.1f StaminaCost=%.1f"),
		*PlayerCharacter->GetName(),
		*TargetResource->GetName(),
		*UEnum::GetValueAsString(CurrentToolType),
		GatherDamage,
		GatherStaminaCost);
}

void UPWPlayerGatherComponent::FinishGatherAction()
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		return;
	}

	if (UPWPlayerActionComponent* ActionComponent = PlayerCharacter->GetActionComponent())
	{
		ActionComponent->FinishActionAuthority(EPWPlayerActionState::Gathering);
	}
}
