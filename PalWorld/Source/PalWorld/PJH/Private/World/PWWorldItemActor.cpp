// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/PWWorldItemActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Interfaces/PW_ItemReceiver.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "Player/Data/PWItemDataAsset.h"
#include "PWInteractableTargetComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
FVector MakeRandomDropImpulse(float MinHorizontalImpulse, float MaxHorizontalImpulse, float MinUpwardImpulse, float MaxUpwardImpulse)
{
	const FVector2D RandomDirection = FMath::RandPointInCircle(1.f).GetSafeNormal();
	const float HorizontalImpulse = FMath::FRandRange(FMath::Max(0.f, MinHorizontalImpulse), FMath::Max(MinHorizontalImpulse, MaxHorizontalImpulse));
	const float UpwardImpulse = FMath::FRandRange(FMath::Max(0.f, MinUpwardImpulse), FMath::Max(MinUpwardImpulse, MaxUpwardImpulse));
	return FVector(RandomDirection.X * HorizontalImpulse, RandomDirection.Y * HorizontalImpulse, UpwardImpulse);
}
}

APWWorldItemActor::APWWorldItemActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;
	SetReplicateMovement(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(SceneRoot);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);
	ItemMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	ItemMesh->SetGenerateOverlapEvents(false);
	ItemMesh->SetRenderCustomDepth(false);
	ItemMesh->SetCustomDepthStencilValue(HighlightStencilValue);

	OutlineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OutlineMesh"));
	OutlineMesh->SetupAttachment(ItemMesh);
	OutlineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OutlineMesh->SetGenerateOverlapEvents(false);
	OutlineMesh->SetCastShadow(false);
	OutlineMesh->SetHiddenInGame(true);
	OutlineMesh->SetVisibility(false, true);
	OutlineMesh->SetRelativeScale3D(FVector(OutlineScale));
	OutlineMesh->SetReverseCulling(true);

	InteractableTarget = CreateDefaultSubobject<UPWInteractableTargetComponent>(TEXT("InteractableTarget"));
	InteractableTarget->SetInteractionRadius(PickupRadius);
	InteractableTarget->SetPromptText(NSLOCTEXT("PWWorldItem", "PickupPrompt", "Pick Up"));
	InteractableTarget->SetPriority(200);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultOutlineMaterial(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (DefaultOutlineMaterial.Succeeded())
	{
		OutlineMaterial = DefaultOutlineMaterial.Object;
	}
}

void APWWorldItemActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() && bDropPhysicsActive && ItemMesh && ItemMesh->IsSimulatingPhysics())
	{
		UpdateReplicatedDropVisualTransform();
		SetActorLocation(GetItemWorldLocation());
		ForceNetUpdate();
	}
}

void APWWorldItemActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoCollectTimerHandle);
		World->GetTimerManager().ClearTimer(DropSettleTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void APWWorldItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APWWorldItemActor, ItemData);
	DOREPLIFETIME(APWWorldItemActor, ItemId);
	DOREPLIFETIME(APWWorldItemActor, Count);
	DOREPLIFETIME(APWWorldItemActor, PreferredReceiver);
	DOREPLIFETIME(APWWorldItemActor, ReplicatedDropVisualTransform);
	DOREPLIFETIME(APWWorldItemActor, bHasReplicatedDropVisualTransform);
}

void APWWorldItemActor::InitializeWorldItem(UPWItemDataAsset* InItemData, FName InItemId, int32 InCount, AActor* InPreferredReceiver, const FVector& InDropImpulse)
{
	if (!HasAuthority())
	{
		return;
	}

	ItemData = InItemData;
	ItemId = InItemData ? InItemData->GetItemId() : InItemId;
	Count = FMath::Max(0, InCount);
	PreferredReceiver = InPreferredReceiver;

	RefreshVisuals();
	UpdateReplicatedDropVisualTransform();
	StartDropPhysics(InDropImpulse);

	if (DroppedItemLifeSeconds > 0.f)
	{
		SetLifeSpan(DroppedItemLifeSeconds);
	}

	ForceNetUpdate();
}

FVector APWWorldItemActor::ResolveGroundedDropLocation(
	UWorld* World,
	const FVector& DesiredLocation,
	const TArray<AActor*>& IgnoredActors,
	float TraceUpDistance,
	float TraceDownDistance,
	float GroundOffset)
{
	if (!World)
	{
		return DesiredLocation;
	}

	const FVector TraceStart = DesiredLocation + FVector(0.f, 0.f, FMath::Max(0.f, TraceUpDistance));
	const FVector TraceEnd = DesiredLocation - FVector(0.f, 0.f, FMath::Max(0.f, TraceDownDistance));

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWWorldItemGroundTrace), false);
	for (AActor* IgnoredActor : IgnoredActors)
	{
		if (IgnoredActor)
		{
			QueryParams.AddIgnoredActor(IgnoredActor);
		}
	}

	FHitResult HitResult;
	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return HitResult.ImpactPoint + HitResult.ImpactNormal.GetSafeNormal(UE_SMALL_NUMBER, FVector::UpVector) * FMath::Max(0.f, GroundOffset);
	}

	return DesiredLocation;
}

bool APWWorldItemActor::Collect(AActor* Collector)
{
	if (!HasAuthority() || Count <= 0 || ItemId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWWorldItem] Collect rejected. Authority=%s Count=%d ItemId=%s Collector=%s"),
			HasAuthority() ? TEXT("true") : TEXT("false"),
			Count,
			*ItemId.ToString(),
			*GetNameSafe(Collector));
		return false;
	}

	AActor* Receiver = ResolveReceiver(Collector);
	if (!Receiver || !Receiver->GetClass()->ImplementsInterface(UPW_ItemReceiver::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWWorldItem] Collect rejected. No receiver. ItemId=%s Collector=%s PreferredReceiver=%s"),
			*ItemId.ToString(),
			*GetNameSafe(Collector),
			*GetNameSafe(PreferredReceiver.Get()));
		return false;
	}

	if (!IPW_ItemReceiver::Execute_ReceiveItem(Receiver, ItemId, Count))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWWorldItem] Collect rejected. Receiver failed item add. Receiver=%s ItemId=%s Count=%d"),
			*GetNameSafe(Receiver),
			*ItemId.ToString(),
			Count);
		return false;
	}

	UE_LOG(LogTemp, Verbose, TEXT("[PWWorldItem] Collected. Receiver=%s ItemId=%s Count=%d"),
		*GetNameSafe(Receiver),
		*ItemId.ToString(),
		Count);

	Destroy();
	return true;
}

void APWWorldItemActor::SetHighlighted(bool bHighlighted)
{
	if (ItemMesh)
	{
		ItemMesh->SetRenderCustomDepth(bHighlighted);
		ItemMesh->SetCustomDepthStencilValue(HighlightStencilValue);
	}

	if (OutlineMesh)
	{
		OutlineMesh->SetHiddenInGame(!bHighlighted);
		OutlineMesh->SetVisibility(bHighlighted, true);
		OutlineMesh->SetRenderCustomDepth(false);
	}
}

void APWWorldItemActor::StartAutoCollect(float InitialDelay, float InAutoCollectRadius)
{
	if (!HasAuthority())
	{
		return;
	}

	AutoCollectRadius = FMath::Max(0.f, InAutoCollectRadius);

	UWorld* World = GetWorld();
	if (!World || !PreferredReceiver)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWWorldItem] Auto collect not started. World=%s PreferredReceiver=%s ItemId=%s"),
			World ? TEXT("valid") : TEXT("none"),
			*GetNameSafe(PreferredReceiver.Get()),
			*ItemId.ToString());
		return;
	}

	const float DistanceToReceiver = FVector::Dist(PreferredReceiver->GetActorLocation(), GetItemWorldLocation());
	UE_LOG(LogTemp, Verbose, TEXT("[PWWorldItem] Auto collect scheduled. ItemId=%s Count=%d Delay=%.2f Radius=%.1f Distance=%.1f Location=%s Receiver=%s"),
		*ItemId.ToString(),
		Count,
		FMath::Max(0.05f, InitialDelay),
		AutoCollectRadius,
		DistanceToReceiver,
		*GetItemWorldLocation().ToString(),
		*GetNameSafe(PreferredReceiver.Get()));

	World->GetTimerManager().ClearTimer(AutoCollectTimerHandle);
	World->GetTimerManager().SetTimer(
		AutoCollectTimerHandle,
		this,
		&APWWorldItemActor::TryAutoCollect,
		FMath::Max(0.05f, InitialDelay),
		false);
}

bool APWWorldItemActor::CanInteract_Implementation(AActor* Interactor) const
{
	if (!Interactor || Count <= 0 || ItemId.IsNone() || !InteractableTarget || !InteractableTarget->IsInteractionEnabled())
	{
		return false;
	}

	const float Radius = FMath::Max(PickupRadius, InteractableTarget->GetInteractionRadius());
	return FVector::DistSquared(Interactor->GetActorLocation(), GetItemWorldLocation()) <= FMath::Square(Radius);
}

bool APWWorldItemActor::Interact_Implementation(AActor* Interactor)
{
	return Collect(Interactor);
}

FText APWWorldItemActor::GetInteractionPrompt_Implementation() const
{
	if (ItemData)
	{
		return FText::Format(NSLOCTEXT("PWWorldItem", "PickupNamedPrompt", "Pick Up {0}"), ItemData->GetDisplayName());
	}

	return InteractableTarget ? InteractableTarget->GetPromptText() : NSLOCTEXT("PWWorldItem", "PickupPrompt", "Pick Up");
}

int32 APWWorldItemActor::GetInteractionPriority_Implementation() const
{
	return InteractableTarget ? InteractableTarget->GetPriority() : 200;
}

void APWWorldItemActor::OnRep_ItemData()
{
	RefreshVisuals();
	ApplyReplicatedDropVisualTransform();
}

void APWWorldItemActor::OnRep_ItemState()
{
	RefreshVisuals();
	ApplyReplicatedDropVisualTransform();
}

void APWWorldItemActor::OnRep_DropVisualTransform()
{
	ApplyReplicatedDropVisualTransform();
}

FVector APWWorldItemActor::GetItemWorldLocation() const
{
	return ItemMesh ? ItemMesh->GetComponentLocation() : GetActorLocation();
}

void APWWorldItemActor::RefreshVisuals()
{
	if (!ItemMesh)
	{
		return;
	}

	UStaticMesh* WorldMesh = ItemData ? ItemData->GetWorldMesh() : nullptr;
	if (!WorldMesh && ItemData && ItemData->IsEquippable())
	{
		WorldMesh = ItemData->GetEquipmentStaticMesh();
	}
	const FTransform WorldDropTransform = ItemData ? ItemData->GetWorldDropTransform() : FTransform::Identity;
	ItemMesh->SetStaticMesh(WorldMesh);
	ItemMesh->SetRelativeTransform(WorldDropTransform);
	ItemMesh->SetRenderCustomDepth(false);
	ItemMesh->SetCustomDepthStencilValue(HighlightStencilValue);

	if (OutlineMesh)
	{
		OutlineMesh->SetStaticMesh(WorldMesh);
		OutlineMesh->SetRelativeLocation(FVector::ZeroVector);
		OutlineMesh->SetRelativeRotation(FRotator::ZeroRotator);
		OutlineMesh->SetRelativeScale3D(FVector(OutlineScale));
		OutlineMesh->SetReverseCulling(true);
		OutlineMesh->SetHiddenInGame(true);
		OutlineMesh->SetVisibility(false, true);
		OutlineMesh->SetRenderCustomDepth(false);

		if (OutlineMaterial)
		{
			const int32 MaterialSlotCount = FMath::Max(1, OutlineMesh->GetNumMaterials());
			for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotCount; ++MaterialIndex)
			{
				OutlineMesh->SetMaterial(MaterialIndex, OutlineMaterial);
			}
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("[PWWorldItem] Refresh visuals. Actor=%s ItemId=%s ItemData=%s WorldMesh=%s"),
		*GetName(),
		*ItemId.ToString(),
		*GetNameSafe(ItemData),
		*GetNameSafe(WorldMesh));
}

void APWWorldItemActor::ApplyReplicatedDropVisualTransform()
{
	if (HasAuthority() || !ItemMesh || !bHasReplicatedDropVisualTransform)
	{
		return;
	}

	ItemMesh->SetSimulatePhysics(false);
	ItemMesh->SetWorldTransform(ReplicatedDropVisualTransform);
}

void APWWorldItemActor::UpdateReplicatedDropVisualTransform()
{
	if (!HasAuthority() || !ItemMesh)
	{
		return;
	}

	ReplicatedDropVisualTransform = ItemMesh->GetComponentTransform();
	bHasReplicatedDropVisualTransform = true;
}

void APWWorldItemActor::StartDropPhysics(const FVector& InDropImpulse)
{
	if (!HasAuthority() || !ItemMesh || !bUseDropPhysics || !ItemMesh->GetStaticMesh())
	{
		return;
	}

	const FVector DropImpulse = InDropImpulse.IsNearlyZero()
		? MakeRandomDropImpulse(MinRandomHorizontalImpulse, MaxRandomHorizontalImpulse, MinUpwardImpulse, MaxUpwardImpulse)
		: InDropImpulse;

	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);
	ItemMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	ItemMesh->SetSimulatePhysics(true);
	ItemMesh->WakeRigidBody();
	ItemMesh->AddImpulse(DropImpulse, NAME_None, true);
	UpdateReplicatedDropVisualTransform();
	bDropPhysicsActive = true;
	SetActorTickEnabled(true);

	if (AngularImpulseStrength > 0.f)
	{
		ItemMesh->AddAngularImpulseInDegrees(FMath::VRand() * AngularImpulseStrength, NAME_None, true);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DropSettleTimerHandle);
		World->GetTimerManager().SetTimer(
			DropSettleTimerHandle,
			this,
			&APWWorldItemActor::SettleDropPhysics,
			FMath::Max(0.05f, DropSettleDelay),
			false);
	}
}

void APWWorldItemActor::SettleDropPhysics()
{
	if (!HasAuthority() || !ItemMesh)
	{
		return;
	}

	const FTransform SettledMeshTransform = ItemMesh->GetComponentTransform();
	bDropPhysicsActive = false;
	SetActorTickEnabled(false);
	ItemMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	ItemMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	ItemMesh->SetSimulatePhysics(false);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetActorLocation(SettledMeshTransform.GetLocation());
	ItemMesh->SetWorldTransform(SettledMeshTransform);
	ReplicatedDropVisualTransform = SettledMeshTransform;
	bHasReplicatedDropVisualTransform = true;
	ForceNetUpdate();
}

AActor* APWWorldItemActor::ResolveReceiver(AActor* Collector) const
{
	for (AActor* Candidate = Collector ? Collector : PreferredReceiver.Get(); Candidate != nullptr; Candidate = Candidate->GetOwner())
	{
		if (Candidate->GetClass()->ImplementsInterface(UPW_ItemReceiver::StaticClass()))
		{
			return Candidate;
		}
	}

	for (AActor* Candidate = PreferredReceiver.Get(); Candidate != nullptr; Candidate = Candidate->GetOwner())
	{
		if (Candidate->GetClass()->ImplementsInterface(UPW_ItemReceiver::StaticClass()))
		{
			return Candidate;
		}
	}

	return nullptr;
}

void APWWorldItemActor::TryAutoCollect()
{
	if (!HasAuthority() || Count <= 0 || ItemId.IsNone())
	{
		return;
	}

	AActor* Receiver = PreferredReceiver.Get();
	if (!Receiver)
	{
		return;
	}

	if (FVector::DistSquared(Receiver->GetActorLocation(), GetItemWorldLocation()) <= FMath::Square(AutoCollectRadius))
	{
		Collect(Receiver);
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AutoCollectTimerHandle,
			this,
			&APWWorldItemActor::TryAutoCollect,
			0.25f,
			false);
	}
}
