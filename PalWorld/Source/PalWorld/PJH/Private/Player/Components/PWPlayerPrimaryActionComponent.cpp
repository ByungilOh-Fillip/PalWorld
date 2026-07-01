// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerPrimaryActionComponent.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTags/PW_GameplayTags.h"
#include "Interfaces/PW_HarvestDamageTarget.h"
#include "Interfaces/PW_HarvestInstanceDamageTarget.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPlayerActionComponent.h"
#include "Player/Components/PWPlayerEquipmentComponent.h"
#include "Player/Components/PWPlayerStatComponent.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "Player/Data/PWPrimaryActionDataAsset.h"
#include "Player/UI/PWLocalDamageFloatActor.h"
#include "Resource/PW_HarvestableResourceClusterComponent.h"
#include "Resource/PW_HarvestableResourceComponent.h"
#include "TimerManager.h"

UPWPlayerPrimaryActionComponent::UPWPlayerPrimaryActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPWPlayerPrimaryActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPWPlayerPrimaryActionComponent, CurrentToolType);
}

void UPWPlayerPrimaryActionComponent::TryStartPrimaryAction()
{
	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ZeroVector;
	if (!GetView(ViewLocation, ViewDirection))
	{
		BP_OnPrimaryActionFailed();
		return;
	}

	FHitResult HitResult;
	if (!FindTargetFromView(HitResult))
	{
		BP_OnPrimaryActionFailed();
		return;
	}

	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerRequestPrimaryAction(FVector_NetQuantize(ViewLocation), FVector_NetQuantizeNormal(ViewDirection));
		return;
	}

	StartAuthority(HitResult, ViewDirection);
}

void UPWPlayerPrimaryActionComponent::SetToolType(EPWToolType NewToolType)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerSetToolType(NewToolType);
		return;
	}

	CurrentToolType = NewToolType;
}

EPWToolType UPWPlayerPrimaryActionComponent::GetCurrentToolType() const
{
	return ResolveCurrentToolType();
}

void UPWPlayerPrimaryActionComponent::ServerRequestPrimaryAction_Implementation(FVector_NetQuantize ViewLocation, FVector_NetQuantizeNormal ViewDirection)
{
	if (!IsViewLocationAllowed(ViewLocation))
	{
		BP_OnPrimaryActionFailed();
		return;
	}

	FHitResult HitResult;
	if (!FindTargetFromViewData(ViewLocation, ViewDirection, HitResult))
	{
		BP_OnPrimaryActionFailed();
		return;
	}

	StartAuthority(HitResult, ViewDirection);
}

void UPWPlayerPrimaryActionComponent::ServerSetToolType_Implementation(EPWToolType NewToolType)
{
	CurrentToolType = NewToolType;
}

void UPWPlayerPrimaryActionComponent::ClientShowDamage_Implementation(float AppliedDamage, FVector_NetQuantize WorldLocation, EPWToolType ToolType, EPWResourceType ResourceType)
{
	ShowDamageLocal(AppliedDamage, WorldLocation, ToolType, ResourceType);
}

void UPWPlayerPrimaryActionComponent::OnRep_CurrentToolType()
{
}

APWPlayerCharacter* UPWPlayerPrimaryActionComponent::GetPlayerCharacter() const
{
	return Cast<APWPlayerCharacter>(GetOwner());
}

const UPWPrimaryActionDataAsset* UPWPlayerPrimaryActionComponent::GetActionData() const
{
	return PrimaryActionData ? PrimaryActionData.Get() : GetDefault<UPWPrimaryActionDataAsset>();
}

float UPWPlayerPrimaryActionComponent::GetTraceDistance() const
{
	return GetActionData()->GetTraceDistance();
}

float UPWPlayerPrimaryActionComponent::GetMaxViewLocationDistance() const
{
	return GetActionData()->GetMaxViewLocationDistance();
}

float UPWPlayerPrimaryActionComponent::GetStaminaCost() const
{
	return GetActionData()->GetStaminaCost();
}

float UPWPlayerPrimaryActionComponent::GetActionDuration() const
{
	return GetActionData()->GetActionDuration();
}

float UPWPlayerPrimaryActionComponent::GetDamageVarianceRatio() const
{
	return GetActionData()->GetDamageVarianceRatio();
}

bool UPWPlayerPrimaryActionComponent::GetView(FVector& OutLocation, FVector& OutDirection) const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	const AController* Controller = PlayerCharacter ? PlayerCharacter->GetController() : nullptr;
	if (!Controller)
	{
		return false;
	}

	FRotator ViewRotation = FRotator::ZeroRotator;
	Controller->GetPlayerViewPoint(OutLocation, ViewRotation);
	OutDirection = ViewRotation.Vector();
	return !OutDirection.IsNearlyZero();
}

bool UPWPlayerPrimaryActionComponent::FindTargetFromView(FHitResult& OutHitResult) const
{
	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ZeroVector;
	return GetView(ViewLocation, ViewDirection)
		&& FindTarget(ViewLocation, ViewDirection, OutHitResult);
}

bool UPWPlayerPrimaryActionComponent::FindTargetFromViewData(const FVector& ViewLocation, const FVector& ViewDirection, FHitResult& OutHitResult) const
{
	return IsViewLocationAllowed(ViewLocation)
		&& FindTarget(ViewLocation, ViewDirection, OutHitResult);
}

bool UPWPlayerPrimaryActionComponent::FindTarget(const FVector& TraceStart, const FVector& TraceDirection, FHitResult& OutHitResult) const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || TraceDirection.IsNearlyZero())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWPrimaryActionTrace), false, PlayerCharacter);
	QueryParams.AddIgnoredActor(PlayerCharacter);

	const FVector TraceEnd = TraceStart + TraceDirection.GetSafeNormal() * GetTraceDistance();
	return World->LineTraceSingleByChannel(OutHitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
}

bool UPWPlayerPrimaryActionComponent::IsViewLocationAllowed(const FVector& ViewLocation) const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	return PlayerCharacter
		&& FVector::DistSquared(PlayerCharacter->GetActorLocation(), ViewLocation) <= FMath::Square(GetMaxViewLocationDistance());
}

bool UPWPlayerPrimaryActionComponent::CanDamageTarget(const FHitResult& HitResult) const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	AActor* TargetActor = HitResult.GetActor();
	if (!PlayerCharacter || !TargetActor)
	{
		return false;
	}

	if (const UPW_HarvestableResourceComponent* ResourceComponent = TargetActor->FindComponentByClass<UPW_HarvestableResourceComponent>())
	{
		if (ResourceComponent->IsDepleted())
		{
			return false;
		}
	}
	else if (const UPW_HarvestableResourceClusterComponent* ClusterComponent = TargetActor->FindComponentByClass<UPW_HarvestableResourceClusterComponent>())
	{
		if (HitResult.Item == INDEX_NONE || ClusterComponent->IsInstanceDepleted(HitResult.Item))
		{
			return false;
		}
	}
	else if (!TargetActor->GetClass()->ImplementsInterface(UPW_HarvestDamageTarget::StaticClass())
		&& !TargetActor->GetClass()->ImplementsInterface(UPW_HarvestInstanceDamageTarget::StaticClass()))
	{
		return false;
	}

	const UPWPlayerActionComponent* ActionComponent = PlayerCharacter->GetActionComponent();
	if (ActionComponent && !ActionComponent->CanStartAction(EPWPlayerActionState::PrimaryAction))
	{
		return false;
	}

	const FVector TargetLocation = HitResult.ImpactPoint.IsNearlyZero()
		? TargetActor->GetActorLocation()
		: FVector(HitResult.ImpactPoint);
	return FVector::DistSquared(PlayerCharacter->GetActorLocation(), TargetLocation) <= FMath::Square(GetTraceDistance() + 50.f);
}

EPWToolType UPWPlayerPrimaryActionComponent::ResolveCurrentToolType() const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	const UPWPlayerEquipmentComponent* EquipmentComponent = PlayerCharacter ? PlayerCharacter->GetEquipmentComponent() : nullptr;
	return EquipmentComponent ? EquipmentComponent->GetSelectedToolType() : CurrentToolType;
}

EPWResourceType UPWPlayerPrimaryActionComponent::ResolveResourceType(const FHitResult& HitResult) const
{
	const AActor* TargetActor = HitResult.GetActor();
	if (!TargetActor)
	{
		return EPWResourceType::Tree;
	}

	if (const UPW_HarvestableResourceComponent* ResourceComponent = TargetActor->FindComponentByClass<UPW_HarvestableResourceComponent>())
	{
		return ResolveResourceTypeFromReward(ResourceComponent->GetRewardName(), ResourceComponent->GetRequiredWorkTag());
	}

	if (const UPW_HarvestableResourceClusterComponent* ClusterComponent = TargetActor->FindComponentByClass<UPW_HarvestableResourceClusterComponent>())
	{
		return ResolveResourceTypeFromReward(NAME_None, ClusterComponent->GetRequiredWorkTag());
	}

	const FString ActorName = TargetActor->GetName();
	if (ActorName.Contains(TEXT("Rock")) || ActorName.Contains(TEXT("Stone")))
	{
		return EPWResourceType::Stone;
	}

	return EPWResourceType::Tree;
}

EPWResourceType UPWPlayerPrimaryActionComponent::ResolveResourceTypeFromReward(FName RewardName, const FGameplayTag& RequiredWorkTag) const
{
	if (RequiredWorkTag.MatchesTagExact(PW_GameplayTags::Work_Mining))
	{
		return EPWResourceType::Stone;
	}

	if (RequiredWorkTag.MatchesTagExact(PW_GameplayTags::Work_Lumbering))
	{
		return EPWResourceType::Tree;
	}

	const FString RewardString = RewardName.ToString();
	if (RewardString.Contains(TEXT("Stone")) || RewardString.Contains(TEXT("Rock")) || RewardString.Contains(TEXT("Ore")))
	{
		return EPWResourceType::Stone;
	}

	return EPWResourceType::Tree;
}

float UPWPlayerPrimaryActionComponent::ResolveDamage(EPWResourceType ResourceType) const
{
	return GetActionData()->GetDamage(ResolveCurrentToolType(), ResourceType);
}

float UPWPlayerPrimaryActionComponent::ApplyDamageVariance(float BaseDamage) const
{
	const float VarianceRatio = GetDamageVarianceRatio();
	if (BaseDamage <= 0.f || VarianceRatio <= 0.f)
	{
		return BaseDamage;
	}

	const float MinMultiplier = FMath::Max(0.f, 1.f - VarianceRatio);
	return BaseDamage * FMath::FRandRange(MinMultiplier, 1.f + VarianceRatio);
}

bool UPWPlayerPrimaryActionComponent::ApplyDamageToTarget(const FHitResult& HitResult, float DamageAmount, float& OutAppliedDamage) const
{
	OutAppliedDamage = 0.f;

	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	AActor* TargetActor = HitResult.GetActor();
	if (!PlayerCharacter || !TargetActor || DamageAmount <= 0.f)
	{
		return false;
	}

	if (HitResult.Item != INDEX_NONE && TargetActor->GetClass()->ImplementsInterface(UPW_HarvestInstanceDamageTarget::StaticClass()))
	{
		const bool bApplied = IPW_HarvestInstanceDamageTarget::Execute_ApplyHarvestDamageToInstance(
			TargetActor,
			HitResult.Item,
			DamageAmount,
			PlayerCharacter);
		OutAppliedDamage = bApplied ? DamageAmount : 0.f;
		return bApplied;
	}

	if (TargetActor->GetClass()->ImplementsInterface(UPW_HarvestDamageTarget::StaticClass()))
	{
		const bool bApplied = IPW_HarvestDamageTarget::Execute_ApplyHarvestDamage(TargetActor, DamageAmount, PlayerCharacter);
		OutAppliedDamage = bApplied ? DamageAmount : 0.f;
		return bApplied;
	}

	return false;
}

void UPWPlayerPrimaryActionComponent::StartAuthority(const FHitResult& HitResult, const FVector& ActionDirection)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	AActor* TargetActor = HitResult.GetActor();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || !CanDamageTarget(HitResult))
	{
		BP_OnPrimaryActionFailed();
		return;
	}

	UPWPlayerActionComponent* ActionComponent = PlayerCharacter->GetActionComponent();
	if (ActionComponent && !ActionComponent->TryStartActionAuthority(EPWPlayerActionState::PrimaryAction))
	{
		BP_OnPrimaryActionFailed();
		return;
	}

	auto FinishPrimaryActionState = [ActionComponent]()
	{
		if (ActionComponent)
		{
			ActionComponent->FinishActionAuthority(EPWPlayerActionState::PrimaryAction);
		}
	};

	FVector FlatDirection = ActionDirection;
	FlatDirection.Z = 0.f;
	if (!FlatDirection.IsNearlyZero())
	{
		// 서버도 액션 방향을 확정해 다른 클라이언트가 보는 캐릭터 방향을 맞춘다.
		PlayerCharacter->SetActorRotation(FRotator(0.f, FlatDirection.Rotation().Yaw, 0.f));
	}

	UPWPlayerStatComponent* StatComponent = PlayerCharacter->GetStatComponent();
	const float ActionStaminaCost = GetStaminaCost();
	if (StatComponent && !StatComponent->TryConsumeStamina(ActionStaminaCost))
	{
		FinishPrimaryActionState();
		BP_OnPrimaryActionFailed();
		return;
	}

	const EPWToolType ToolType = ResolveCurrentToolType();
	const EPWResourceType ResourceType = ResolveResourceType(HitResult);
	const float DamageAmount = ApplyDamageVariance(ResolveDamage(ResourceType));

	float AppliedDamage = 0.f;
	if (!ApplyDamageToTarget(HitResult, DamageAmount, AppliedDamage))
	{
		FinishPrimaryActionState();
		BP_OnPrimaryActionFailed();
		return;
	}

	BP_OnPrimaryActionStarted(TargetActor, ToolType);

	const FVector DamageLocation = HitResult.ImpactPoint.IsNearlyZero()
		? TargetActor->GetActorLocation() + FVector(0.f, 0.f, 120.f)
		: FVector(HitResult.ImpactPoint) + FVector(0.f, 0.f, 60.f);

	if (PlayerCharacter->IsLocallyControlled())
	{
		// 단독 실행/리스슨 서버의 로컬 플레이어는 소유 클라 RPC를 기다리지 않고 즉시 표시한다.
		ShowDamageLocal(AppliedDamage, DamageLocation, ToolType, ResourceType);
	}
	else
	{
		ClientShowDamage(AppliedDamage, FVector_NetQuantize(DamageLocation), ToolType, ResourceType);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActionTimerHandle);
		World->GetTimerManager().SetTimer(ActionTimerHandle, this, &UPWPlayerPrimaryActionComponent::FinishAction, GetActionDuration(), false);
	}

}

void UPWPlayerPrimaryActionComponent::FinishAction()
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		return;
	}

	if (UPWPlayerActionComponent* ActionComponent = PlayerCharacter->GetActionComponent())
	{
		ActionComponent->FinishActionAuthority(EPWPlayerActionState::PrimaryAction);
	}
}

void UPWPlayerPrimaryActionComponent::ShowDamageLocal(float AppliedDamage, const FVector& WorldLocation, EPWToolType ToolType, EPWResourceType ResourceType)
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

	BP_OnLocalDamageFloat(AppliedDamage, WorldLocation, ToolType, ResourceType);
}
