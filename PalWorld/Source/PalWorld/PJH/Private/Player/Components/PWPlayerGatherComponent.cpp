// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerGatherComponent.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPlayerActionComponent.h"
#include "Player/Components/PWPlayerEquipmentComponent.h"
#include "Player/Components/PWPlayerStatComponent.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "Player/UI/PWLocalDamageFloatActor.h"
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
	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ZeroVector;
	if (!GetGatherView(ViewLocation, ViewDirection))
	{
		BP_OnGatherFailed();
		return;
	}

	APWGatherableResourceActor* TargetResource = FindGatherTargetFromView();
	if (!TargetResource)
	{
		BP_OnGatherFailed();
		return;
	}

	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerRequestGatherFromView(FVector_NetQuantize(ViewLocation), FVector_NetQuantizeNormal(ViewDirection));
		return;
	}

	GatherAuthority(TargetResource, ViewDirection);
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

EPWToolType UPWPlayerGatherComponent::GetCurrentToolType() const
{
	return ResolveCurrentToolType();
}

void UPWPlayerGatherComponent::ServerRequestGatherFromView_Implementation(FVector_NetQuantize RequestedViewLocation, FVector_NetQuantizeNormal RequestedViewDirection)
{
	if (!IsGatherViewLocationAllowed(RequestedViewLocation))
	{
		BP_OnGatherFailed();
		return;
	}

	APWGatherableResourceActor* TargetResource = FindGatherTargetFromViewData(RequestedViewLocation, RequestedViewDirection);
	if (!TargetResource)
	{
		BP_OnGatherFailed();
		return;
	}

	GatherAuthority(TargetResource, RequestedViewDirection);
}

void UPWPlayerGatherComponent::ServerRequestEquipTool_Implementation(EPWToolType NewToolType)
{
	CurrentToolType = NewToolType;
}

void UPWPlayerGatherComponent::ClientShowGatherDamage_Implementation(float AppliedDamage, FVector_NetQuantize WorldLocation, EPWToolType ToolType, EPWResourceType ResourceType)
{
	ShowGatherDamageLocal(AppliedDamage, WorldLocation, ToolType, ResourceType);
}

void UPWPlayerGatherComponent::OnRep_CurrentToolType()
{
}

APWPlayerCharacter* UPWPlayerGatherComponent::GetPlayerCharacter() const
{
	return Cast<APWPlayerCharacter>(GetOwner());
}

bool UPWPlayerGatherComponent::GetGatherView(FVector& OutViewLocation, FVector& OutViewDirection) const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		return false;
	}

	const AController* Controller = PlayerCharacter->GetController();
	if (!Controller)
	{
		return false;
	}

	FRotator ViewRotation = FRotator::ZeroRotator;
	Controller->GetPlayerViewPoint(OutViewLocation, ViewRotation);
	OutViewDirection = ViewRotation.Vector();

	return !OutViewDirection.IsNearlyZero();
}

void UPWPlayerGatherComponent::ShowGatherDamageLocal(float AppliedDamage, const FVector& WorldLocation, EPWToolType ToolType, EPWResourceType ResourceType)
{
	if (UWorld* World = GetWorld())
	{
		UClass* FloatActorClass = DamageFloatActorClass
			? DamageFloatActorClass.Get()
			: APWLocalDamageFloatActor::StaticClass();

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (APWLocalDamageFloatActor* DamageFloatActor = World->SpawnActor<APWLocalDamageFloatActor>(
			FloatActorClass,
			WorldLocation,
			FRotator::ZeroRotator,
			SpawnParameters))
		{
			DamageFloatActor->InitializeDamageFloat(AppliedDamage);
		}
	}

	BP_OnLocalGatherDamageFloat(AppliedDamage, WorldLocation, ToolType, ResourceType);
}

APWGatherableResourceActor* UPWPlayerGatherComponent::FindGatherTargetFromView() const
{
	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ZeroVector;
	if (!GetGatherView(ViewLocation, ViewDirection))
	{
		return nullptr;
	}

	return FindGatherTarget(ViewLocation, ViewDirection);
}

APWGatherableResourceActor* UPWPlayerGatherComponent::FindGatherTargetFromViewData(const FVector& ViewLocation, const FVector& ViewDirection) const
{
	if (!IsGatherViewLocationAllowed(ViewLocation))
	{
		return nullptr;
	}

	return FindGatherTarget(ViewLocation, ViewDirection);
}

APWGatherableResourceActor* UPWPlayerGatherComponent::FindGatherTarget(const FVector& TraceStart, const FVector& TraceDirection) const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || TraceDirection.IsNearlyZero())
	{
		return nullptr;
	}

	const FVector TraceEnd = TraceStart + TraceDirection.GetSafeNormal() * GatherTraceDistance;

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

bool UPWPlayerGatherComponent::IsGatherViewLocationAllowed(const FVector& ViewLocation) const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		return false;
	}

	return FVector::DistSquared(PlayerCharacter->GetActorLocation(), ViewLocation) <= FMath::Square(MaxGatherViewLocationDistance);
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
	const EPWToolType ToolType = ResolveCurrentToolType();

	switch (ResourceType)
	{
	case EPWResourceType::Tree:
		switch (ToolType)
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
		switch (ToolType)
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

EPWToolType UPWPlayerGatherComponent::ResolveCurrentToolType() const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (PlayerCharacter)
	{
		if (const UPWPlayerEquipmentComponent* EquipmentComponent = PlayerCharacter->GetEquipmentComponent())
		{
			return EquipmentComponent->GetSelectedToolType();
		}
	}

	return CurrentToolType;
}

float UPWPlayerGatherComponent::ApplyDamageVariance(float BaseDamage) const
{
	if (BaseDamage <= 0.f || DamageVarianceRatio <= 0.f)
	{
		return BaseDamage;
	}

	const float MinMultiplier = FMath::Max(0.f, 1.f - DamageVarianceRatio);
	const float MaxMultiplier = 1.f + DamageVarianceRatio;
	return BaseDamage * FMath::FRandRange(MinMultiplier, MaxMultiplier);
}

void UPWPlayerGatherComponent::GatherAuthority(APWGatherableResourceActor* TargetResource, const FVector& ActionDirection)
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

	FVector FlatActionDirection = ActionDirection;
	FlatActionDirection.Z = 0.f;
	if (!FlatActionDirection.IsNearlyZero())
	{
		// 서버도 액션 방향을 확정해 다른 클라이언트가 보는 캐릭터 방향을 맞춘다.
		PlayerCharacter->SetActorRotation(FRotator(0.f, FlatActionDirection.Rotation().Yaw, 0.f));
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

	const EPWToolType ToolType = ResolveCurrentToolType();
	const EPWResourceType ResourceType = TargetResource->GetResourceType();
	const float GatherDamage = ApplyDamageVariance(ResolveGatherDamage(ResourceType));
	float AppliedDamage = 0.f;
	if (!TargetResource->ApplyGatherDamage(PlayerCharacter, GatherDamage, ToolType, AppliedDamage))
	{
		if (ActionComponent)
		{
			ActionComponent->FinishActionAuthority(EPWPlayerActionState::Gathering);
		}
		BP_OnGatherFailed();
		return;
	}

	BP_OnGatherStarted(TargetResource, ToolType);
	const FVector DamageFloatLocation = TargetResource->GetActorLocation() + FVector(0.f, 0.f, 120.f);
	if (PlayerCharacter->IsLocallyControlled())
	{
		// 단독 실행/리스슨 서버의 로컬 플레이어는 소유 클라 RPC를 기다리지 않고 즉시 표시한다.
		ShowGatherDamageLocal(AppliedDamage, DamageFloatLocation, ToolType, ResourceType);
	}
	else
	{
		ClientShowGatherDamage(
			AppliedDamage,
			FVector_NetQuantize(DamageFloatLocation),
			ToolType,
			ResourceType);
	}

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
		*UEnum::GetValueAsString(ToolType),
		AppliedDamage,
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
