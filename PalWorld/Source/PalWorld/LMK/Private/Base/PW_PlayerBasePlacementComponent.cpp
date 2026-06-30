#include "Base/PW_PlayerBasePlacementComponent.h"

#include "Base/PW_BaseCampActor.h"
#include "Base/PW_BaseCampSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

UPW_PlayerBasePlacementComponent::UPW_PlayerBasePlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void UPW_PlayerBasePlacementComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UPW_PlayerBasePlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bPlacementModeActive)
	{
		UpdatePlacementPreview();
		DrawPlacementPreview();
	}
}

void UPW_PlayerBasePlacementComponent::TogglePlacementMode()
{
	if (!bPlacementModeActive)
	{
		SetPlacementModeActive(true);
		return;
	}

	ConfirmPlacement();
}

void UPW_PlayerBasePlacementComponent::ConfirmPlacement()
{
	if (!bPlacementModeActive)
	{
		return;
	}

	UpdatePlacementPreview();
	if (!bCanPlaceAtCurrentPreview || BaseCampClass == nullptr)
	{
		return;
	}

	ServerRequestPlaceBaseCamp(CurrentPlacementTransform);
	SetPlacementModeActive(false);
}

void UPW_PlayerBasePlacementComponent::CancelPlacement()
{
	SetPlacementModeActive(false);
}

void UPW_PlayerBasePlacementComponent::ServerRequestPlaceBaseCamp_Implementation(FTransform PlacementTransform)
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (Owner == nullptr || World == nullptr || BaseCampClass == nullptr)
	{
		return;
	}

	if (!CanPlaceAtTransform(PlacementTransform))
	{
		return;
	}

	APW_BaseCampActor* SpawnedBaseCamp = SpawnBaseCampAuthority(PlacementTransform);
	if (SpawnedBaseCamp == nullptr)
	{
		return;
	}

	FPW_BaseOwnerId OwnerId;
	OwnerId.OwnerType = EPW_BaseOwnerType::Player;
	OwnerId.OwnerId = ResolveOwnerIdString();
	SpawnedBaseCamp->SetBaseOwnerId(OwnerId);
}

void UPW_PlayerBasePlacementComponent::SetPlacementModeActive(bool bNewActive)
{
	bPlacementModeActive = bNewActive;
	SetComponentTickEnabled(bPlacementModeActive);

	if (bPlacementModeActive)
	{
		UpdatePlacementPreview();
	}
}

void UPW_PlayerBasePlacementComponent::UpdatePlacementPreview()
{
	FTransform PlacementTransform;
	if (!ResolvePlacementTransform(PlacementTransform))
	{
		bCanPlaceAtCurrentPreview = false;
		return;
	}

	CurrentPlacementTransform = PlacementTransform;
	bCanPlaceAtCurrentPreview = CanPlaceAtTransform(CurrentPlacementTransform);
}

bool UPW_PlayerBasePlacementComponent::ResolvePlacementView(FVector& OutViewLocation, FVector& OutViewDirection) const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const AController* Controller = OwnerPawn != nullptr ? OwnerPawn->GetController() : nullptr;
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController != nullptr)
	{
		FRotator ViewRotation;
		PlayerController->GetPlayerViewPoint(OutViewLocation, ViewRotation);
		OutViewDirection = ViewRotation.Vector();
		return true;
	}

	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}

	OutViewLocation = Owner->GetActorLocation();
	OutViewDirection = Owner->GetActorForwardVector();
	return true;
}

bool UPW_PlayerBasePlacementComponent::ResolvePlacementTransform(FTransform& OutPlacementTransform) const
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (World == nullptr || Owner == nullptr)
	{
		return false;
	}

	FVector ViewLocation;
	FVector ViewDirection;
	if (!ResolvePlacementView(ViewLocation, ViewDirection))
	{
		return false;
	}

	const FVector TraceEnd = ViewLocation + ViewDirection.GetSafeNormal() * PlacementTraceDistance;
	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWBasePlacementTrace), false, Owner);
	const bool bHit = World->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, PlacementTraceChannel, QueryParams);
	const FVector PlacementLocation = bHit ? HitResult.ImpactPoint : TraceEnd;

	FRotator PlacementRotation = Owner->GetActorRotation();
	PlacementRotation.Pitch = 0.0f;
	PlacementRotation.Roll = 0.0f;
	OutPlacementTransform = FTransform(PlacementRotation, PlacementLocation);
	return true;
}

bool UPW_PlayerBasePlacementComponent::CanPlaceAtTransform(const FTransform& PlacementTransform) const
{
	const UWorld* World = GetWorld();
	const UPW_BaseCampSubsystem* BaseCampSubsystem = World != nullptr ? World->GetSubsystem<UPW_BaseCampSubsystem>() : nullptr;
	return BaseCampSubsystem != nullptr
		&& BaseCampSubsystem->CanPlaceBaseCampAtLocation(PlacementTransform.GetLocation(), PreviewCampRadius);
}

void UPW_PlayerBasePlacementComponent::DrawPlacementPreview() const
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	const UWorld* World = GetWorld();
	if (World == nullptr || World->IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	DrawDebugSphere(
		World,
		CurrentPlacementTransform.GetLocation(),
		PreviewCampRadius,
		64,
		bCanPlaceAtCurrentPreview ? FColor::Blue : FColor::Red,
		false,
		PreviewDrawLifetime,
		0,
		PreviewDrawThickness);
#endif
}

APW_BaseCampActor* UPW_PlayerBasePlacementComponent::SpawnBaseCampAuthority(const FTransform& PlacementTransform)
{
	UWorld* World = GetWorld();
	if (World == nullptr || BaseCampClass == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	return World->SpawnActor<APW_BaseCampActor>(BaseCampClass, PlacementTransform, SpawnParameters);
}

FString UPW_PlayerBasePlacementComponent::ResolveOwnerIdString() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const APlayerState* PlayerState = OwnerPawn != nullptr ? OwnerPawn->GetPlayerState() : nullptr;
	if (PlayerState != nullptr && PlayerState->GetPlayerId() != INDEX_NONE)
	{
		return FString::FromInt(PlayerState->GetPlayerId());
	}

	return GetNameSafe(GetOwner());
}
