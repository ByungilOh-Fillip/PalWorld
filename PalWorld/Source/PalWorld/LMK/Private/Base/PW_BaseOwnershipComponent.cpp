#include "Base/PW_BaseOwnershipComponent.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

UPW_BaseOwnershipComponent::UPW_BaseOwnershipComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

bool UPW_BaseOwnershipComponent::CanAccess(const FPW_BaseOwnerId& BaseOwnerId, const FPW_BaseOwnerId& RequesterOwnerId) const
{
	return BaseOwnerId.IsValid() && BaseOwnerId == RequesterOwnerId;
}

bool UPW_BaseOwnershipComponent::CanModify(const FPW_BaseOwnerId& BaseOwnerId, const FPW_BaseOwnerId& RequesterOwnerId) const
{
	return CanAccess(BaseOwnerId, RequesterOwnerId);
}

bool UPW_BaseOwnershipComponent::CanUseStorage(const FPW_BaseOwnerId& BaseOwnerId, const FPW_BaseOwnerId& RequesterOwnerId) const
{
	return CanAccess(BaseOwnerId, RequesterOwnerId);
}

bool UPW_BaseOwnershipComponent::CanAssignPal(const FPW_BaseOwnerId& BaseOwnerId, const FPW_BaseOwnerId& RequesterOwnerId) const
{
	return CanModify(BaseOwnerId, RequesterOwnerId);
}

FPW_BaseOwnerId UPW_BaseOwnershipComponent::MakeOwnerIdFromPlayerController(const APlayerController* PlayerController) const
{
	FPW_BaseOwnerId OwnerId;
	OwnerId.OwnerType = EPW_BaseOwnerType::Player;

	const APlayerState* PlayerState = PlayerController != nullptr ? PlayerController->PlayerState : nullptr;
	if (PlayerState != nullptr)
	{
		OwnerId.OwnerId = PlayerState->GetPlayerId() != INDEX_NONE
			? FString::FromInt(PlayerState->GetPlayerId())
			: PlayerState->GetPlayerName();
	}

	return OwnerId;
}
