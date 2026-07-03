// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerPrimaryActionComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTags/PW_GameplayTags.h"
#include "Interfaces/PW_HarvestDamageTarget.h"
#include "Interfaces/PW_HarvestInstanceDamageTarget.h"
#include "Net/UnrealNetwork.h"
#include "Player/Animation/PWAnimNotify_PrimaryActionHit.h"
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
	FindTargetFromView(HitResult);

	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerRequestPrimaryAction(FVector_NetQuantize(ViewLocation), FVector_NetQuantizeNormal(ViewDirection));
		return;
	}

	StartAuthority(HitResult, ViewDirection);
}

void UPWPlayerPrimaryActionComponent::TryStopPrimaryAction()
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerRequestStopPrimaryAction();
		return;
	}

	StopAuthority();
}

void UPWPlayerPrimaryActionComponent::HandlePrimaryActionHitNotify()
{
	// 노티파이 자체는 타이밍 마커로만 사용한다.
	// 실제 판정은 서버가 몽타주의 노티파이 시간을 읽어 예약한 타이머에서 처리한다.
	UE_LOG(LogTemp, Verbose, TEXT("[PWPrimaryAction] PrimaryActionHit notify received. Server scheduled timer handles damage."));
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
	FindTargetFromViewData(ViewLocation, ViewDirection, HitResult);

	StartAuthority(HitResult, ViewDirection);
}

void UPWPlayerPrimaryActionComponent::ServerRequestStopPrimaryAction_Implementation()
{
	StopAuthority();
}

void UPWPlayerPrimaryActionComponent::ServerSetToolType_Implementation(EPWToolType NewToolType)
{
	CurrentToolType = NewToolType;
}

void UPWPlayerPrimaryActionComponent::ClientShowDamage_Implementation(float AppliedDamage, FVector_NetQuantize WorldLocation, EPWToolType ToolType, EPWResourceType ResourceType)
{
	ShowDamageLocal(AppliedDamage, WorldLocation, ToolType, ResourceType);
}

void UPWPlayerPrimaryActionComponent::MulticastPlayPrimaryActionAnimation_Implementation(EPWToolType ToolType)
{
	PlayPrimaryActionAnimation(ToolType);
}

void UPWPlayerPrimaryActionComponent::MulticastStopPrimaryActionAnimation_Implementation(EPWToolType ToolType)
{
	StopPrimaryActionAnimation(ToolType);
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

float UPWPlayerPrimaryActionComponent::GetMinHarvestHitInterval() const
{
	return GetActionData()->GetMinHarvestHitInterval();
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

bool UPWPlayerPrimaryActionComponent::ApplyPrimaryActionHitAuthority(const FVector& ViewLocation, const FVector& ViewDirection)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || !bPrimaryActionHeld)
	{
		return false;
	}

	UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.f;
	if (CurrentTime - LastPrimaryActionHitTime < GetMinHarvestHitInterval())
	{
		return false;
	}

	if (!IsViewLocationAllowed(ViewLocation))
	{
		return false;
	}

	LastPrimaryActionHitTime = CurrentTime;

	FHitResult HitResult;
	if (!FindTargetFromViewData(ViewLocation, ViewDirection, HitResult) || !CanDamageTarget(HitResult))
	{
		return true;
	}

	AActor* TargetActor = HitResult.GetActor();
	const EPWToolType ToolType = ResolveCurrentToolType();
	const EPWResourceType ResourceType = ResolveResourceType(HitResult);
	const float DamageAmount = ApplyDamageVariance(ResolveDamage(ResourceType));

	float AppliedDamage = 0.f;
	if (!ApplyDamageToTarget(HitResult, DamageAmount, AppliedDamage) || AppliedDamage <= 0.f || !TargetActor)
	{
		return false;
	}

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

	return true;
}

void UPWPlayerPrimaryActionComponent::ScheduleHarvestHitTimers()
{
	ClearScheduledHitTimers();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const EPWToolType ToolType = ResolveCurrentToolType();
	const UAnimMontage* ActionMontage = GetActionData()->GetPrimaryActionMontage(ToolType);
	TArray<float> HitTimes = GetPrimaryActionHitTimes(ActionMontage);
	if (HitTimes.IsEmpty())
	{
		const float FallbackHitTime = GetActionData()->GetFallbackHitTime(ToolType);
		if (ActionMontage && FallbackHitTime > 0.f)
		{
			HitTimes.Add(FMath::Min(FallbackHitTime, ActionMontage->GetPlayLength()));
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[PWPrimaryAction] Primary action montage has no PrimaryActionHit/Attack1/Attack2 notify. Use fallback hit time %.3fs."),
				HitTimes[0]);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PWPrimaryAction] Primary action montage has no hit notify and no fallback hit time. Damage will not be applied for this swing."));
			return;
		}
	}

	const float PlayRate = FMath::Max(GetActionData()->GetAnimationPlayRate(ToolType), KINDA_SMALL_NUMBER);
	ScheduledHitTimerHandles.Reserve(HitTimes.Num());

	for (const float HitTime : HitTimes)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[PWPrimaryAction] Schedule primary action hit at %.3fs."), HitTime);

		FTimerHandle HitTimerHandle;
		World->GetTimerManager().SetTimer(
			HitTimerHandle,
			this,
			&UPWPlayerPrimaryActionComponent::PerformScheduledPrimaryActionHit,
			FMath::Max(HitTime / PlayRate, 0.01f),
			false);
		ScheduledHitTimerHandles.Add(HitTimerHandle);
	}
}

void UPWPlayerPrimaryActionComponent::ClearScheduledHitTimers()
{
	if (UWorld* World = GetWorld())
	{
		for (FTimerHandle& HitTimerHandle : ScheduledHitTimerHandles)
		{
			World->GetTimerManager().ClearTimer(HitTimerHandle);
		}
	}

	ScheduledHitTimerHandles.Reset();
}

void UPWPlayerPrimaryActionComponent::PerformScheduledPrimaryActionHit()
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || !bPrimaryActionHeld)
	{
		return;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ZeroVector;
	if (!GetView(ViewLocation, ViewDirection))
	{
		FRotator EyeRotation = FRotator::ZeroRotator;
		PlayerCharacter->GetActorEyesViewPoint(ViewLocation, EyeRotation);
		ViewDirection = EyeRotation.Vector();
	}

	ApplyPrimaryActionHitAuthority(ViewLocation, ViewDirection);
}

void UPWPlayerPrimaryActionComponent::StartAuthority(const FHitResult& HitResult, const FVector& ActionDirection)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	AActor* TargetActor = HitResult.GetActor();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		BP_OnPrimaryActionFailed();
		return;
	}

	UPWPlayerActionComponent* ActionComponent = PlayerCharacter->GetActionComponent();
	if (ActionComponent && !ActionComponent->TryStartActionAuthority(EPWPlayerActionState::PrimaryAction))
	{
		bPrimaryActionHeld = false;
		bPrimaryActionFacingLocked = false;
		BP_OnPrimaryActionFailed();
		return;
	}

	if (UPWPlayerStatComponent* StatComponent = PlayerCharacter->GetStatComponent())
	{
		const float ActionStaminaCost = GetStaminaCost();
		if (ActionStaminaCost > 0.f && !StatComponent->TryConsumeStamina(ActionStaminaCost))
		{
			if (ActionComponent)
			{
				ActionComponent->FinishActionAuthority(EPWPlayerActionState::PrimaryAction);
			}

			bPrimaryActionHeld = false;
			bPrimaryActionFacingLocked = false;
			BP_OnPrimaryActionFailed();
			return;
		}
	}

	if (!bPrimaryActionFacingLocked)
	{
		FVector FlatDirection = ActionDirection;
		FlatDirection.Z = 0.f;
		if (!FlatDirection.IsNearlyZero())
		{
			// 좌클릭 유지 중에는 첫 타격에서만 몸 방향을 고정해 반복 루프의 화면 튐을 막는다.
			PlayerCharacter->SetActorRotation(FRotator(0.f, FlatDirection.Rotation().Yaw, 0.f));
		}

		bPrimaryActionFacingLocked = true;
	}

	bPrimaryActionHeld = true;
	LastPrimaryActionHitTime = -FLT_MAX;

	const EPWToolType ToolType = ResolveCurrentToolType();
	BP_OnPrimaryActionStarted(TargetActor, ToolType);
	if (ShouldPlayPrimaryActionAnimation(ToolType))
	{
		MulticastPlayPrimaryActionAnimation(ToolType);
		ScheduleHarvestHitTimers();
	}
	else
	{
		ApplyPrimaryActionHitAuthority(PlayerCharacter->GetActorLocation(), ActionDirection);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActionTimerHandle);
		World->GetTimerManager().SetTimer(
			ActionTimerHandle,
			this,
			&UPWPlayerPrimaryActionComponent::FinishAction,
			GetActionRepeatDuration(ToolType),
			false);
	}

}

void UPWPlayerPrimaryActionComponent::StopAuthority()
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		return;
	}

	bPrimaryActionHeld = false;
	bPrimaryActionFacingLocked = false;
	LastPrimaryActionHitTime = -FLT_MAX;
	ClearScheduledHitTimers();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActionTimerHandle);
	}

	if (UPWPlayerActionComponent* ActionComponent = PlayerCharacter->GetActionComponent())
	{
		ActionComponent->FinishActionAuthority(EPWPlayerActionState::PrimaryAction);
	}

	MulticastStopPrimaryActionAnimation(ResolveCurrentToolType());
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

	if (!bPrimaryActionHeld)
	{
		bPrimaryActionFacingLocked = false;
		ClearScheduledHitTimers();
		return;
	}

	ClearScheduledHitTimers();

	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ZeroVector;
	if (!GetView(ViewLocation, ViewDirection))
	{
		StopAuthority();
		return;
	}

	FHitResult HitResult;
	FindTargetFromView(HitResult);
	StartAuthority(HitResult, ViewDirection);
}

bool UPWPlayerPrimaryActionComponent::ShouldPlayPrimaryActionAnimation(EPWToolType ToolType) const
{
	const UPWPrimaryActionDataAsset* ActionData = GetActionData();
	return ActionData && ActionData->GetPrimaryActionMontage(ToolType);
}

bool UPWPlayerPrimaryActionComponent::IsHarvestHitNotifyName(FName NotifyName) const
{
	static const FName Attack1Name(TEXT("Attack1"));
	static const FName Attack2Name(TEXT("Attack2"));
	static const FName PrimaryActionHitName(TEXT("PrimaryActionHit"));

	return NotifyName == Attack1Name
		|| NotifyName == Attack2Name
		|| NotifyName == PrimaryActionHitName;
}

TArray<float> UPWPlayerPrimaryActionComponent::GetPrimaryActionHitTimes(const UAnimMontage* Montage) const
{
	TArray<float> HitTimes;
	if (!Montage)
	{
		return HitTimes;
	}

	for (const FAnimNotifyEvent& NotifyEvent : Montage->Notifies)
	{
		const bool bNamedHitNotify = IsHarvestHitNotifyName(NotifyEvent.NotifyName);
		const bool bClassHitNotify = NotifyEvent.Notify && NotifyEvent.Notify->IsA<UPWAnimNotify_PrimaryActionHit>();
		if (!bNamedHitNotify && !bClassHitNotify)
		{
			continue;
		}

		const float HitTime = NotifyEvent.GetTriggerTime();
		if (HitTime >= 0.f && HitTime < Montage->GetPlayLength())
		{
			HitTimes.Add(HitTime);
		}
	}

	HitTimes.Sort();
	return HitTimes;
}

float UPWPlayerPrimaryActionComponent::GetActionRepeatDuration(EPWToolType ToolType) const
{
	const UPWPrimaryActionDataAsset* ActionData = GetActionData();
	if (!ActionData)
	{
		return GetActionDuration();
	}

	float RepeatDuration = ActionData->GetActionDurationForTool(ToolType);
	if (const UAnimMontage* ActionMontage = ActionData->GetPrimaryActionMontage(ToolType))
	{
		const float PlayRate = FMath::Max(ActionData->GetAnimationPlayRate(ToolType), KINDA_SMALL_NUMBER);
		const TArray<float> HitTimes = GetPrimaryActionHitTimes(ActionMontage);
		if (!HitTimes.IsEmpty())
		{
			// 반복 주기가 타격 노티파이보다 짧으면 데미지 타이머가 지워지므로, 마지막 타격 직후까지만 보장한다.
			RepeatDuration = FMath::Max(RepeatDuration, HitTimes.Last() / PlayRate + 0.05f);
		}
		else
		{
			RepeatDuration = FMath::Max(RepeatDuration, ActionData->GetFallbackHitTime(ToolType) / PlayRate + 0.05f);
		}
	}

	return FMath::Max(RepeatDuration, 0.05f);
}

void UPWPlayerPrimaryActionComponent::PlayPrimaryActionAnimation(EPWToolType ToolType)
{
	const UPWPrimaryActionDataAsset* ActionData = GetActionData();
	if (!ActionData || !ShouldPlayPrimaryActionAnimation(ToolType))
	{
		return;
	}

	UAnimMontage* ActionMontage = ActionData->GetPrimaryActionMontage(ToolType);
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	USkeletalMeshComponent* CharacterMesh = PlayerCharacter ? PlayerCharacter->GetMesh() : nullptr;
	UAnimInstance* AnimInstance = CharacterMesh ? CharacterMesh->GetAnimInstance() : nullptr;
	if (!ActionMontage || !AnimInstance)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[PWPrimaryAction] Cannot play primary action montage. Montage=%s AnimInstance=%s"),
			ActionMontage ? *ActionMontage->GetName() : TEXT("None"),
			AnimInstance ? *AnimInstance->GetName() : TEXT("None"));
		return;
	}

	const FName SlotName = ActionMontage->SlotAnimTracks.IsEmpty()
		? NAME_None
		: ActionMontage->SlotAnimTracks[0].SlotName;
	const float PlayedDuration = AnimInstance->Montage_Play(ActionMontage, ActionData->GetAnimationPlayRate(ToolType));
	if (PlayedDuration <= 0.f)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[PWPrimaryAction] Montage_Play failed. Montage=%s Slot=%s. Check the character skeleton and ABP Slot node."),
			*ActionMontage->GetName(),
			*SlotName.ToString());
		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[PWPrimaryAction] Play primary action montage. Montage=%s Slot=%s Duration=%.3f"),
		*ActionMontage->GetName(),
		*SlotName.ToString(),
		PlayedDuration);
}

void UPWPlayerPrimaryActionComponent::StopPrimaryActionAnimation(EPWToolType ToolType)
{
	const UPWPrimaryActionDataAsset* ActionData = GetActionData();
	if (!ActionData || !ShouldPlayPrimaryActionAnimation(ToolType))
	{
		return;
	}

	UAnimMontage* ActionMontage = ActionData->GetPrimaryActionMontage(ToolType);
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	USkeletalMeshComponent* CharacterMesh = PlayerCharacter ? PlayerCharacter->GetMesh() : nullptr;
	UAnimInstance* AnimInstance = CharacterMesh ? CharacterMesh->GetAnimInstance() : nullptr;
	if (!ActionMontage || !AnimInstance)
	{
		return;
	}

	AnimInstance->Montage_Stop(ActionData->GetHarvestAnimationBlendOutTime(), ActionMontage);
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
