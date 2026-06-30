#include "Base/PW_BaseWorkTargetRegistryComponent.h"

UPW_BaseWorkTargetRegistryComponent::UPW_BaseWorkTargetRegistryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UPW_BaseWorkTargetRegistryComponent::RegisterWorkTarget(AActor* TargetActor, FName WorkTargetId, FGameplayTag RequiredWorkTag)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || !IsValid(TargetActor) || WorkTargetId.IsNone())
	{
		return;
	}

	UnregisterWorkTarget(TargetActor);

	FPW_WorkTargetEntry Entry;
	Entry.TargetActor = TargetActor;
	Entry.WorkTargetId = WorkTargetId;
	Entry.RequiredWorkTag = RequiredWorkTag;
	WorkTargets.Add(Entry);
}

void UPW_BaseWorkTargetRegistryComponent::UnregisterWorkTarget(AActor* TargetActor)
{
	WorkTargets.RemoveAll([TargetActor](const FPW_WorkTargetEntry& Entry)
	{
		return Entry.TargetActor == TargetActor || !IsValid(Entry.TargetActor);
	});
}

bool UPW_BaseWorkTargetRegistryComponent::FindWorkTargetByTag(FGameplayTag RequiredWorkTag, FPW_WorkTargetEntry& OutEntry) const
{
	for (const FPW_WorkTargetEntry& Entry : WorkTargets)
	{
		if (IsValid(Entry.TargetActor) && Entry.RequiredWorkTag == RequiredWorkTag)
		{
			OutEntry = Entry;
			return true;
		}
	}

	return false;
}
