#include "Base/PW_BaseCampSubsystem.h"

#include "Base/PW_BaseCampActor.h"

void UPW_BaseCampSubsystem::RegisterBaseCamp(APW_BaseCampActor* BaseCamp)
{
	if (IsValid(BaseCamp))
	{
		RegisteredBaseCamps.AddUnique(BaseCamp);
	}
}

void UPW_BaseCampSubsystem::UnregisterBaseCamp(APW_BaseCampActor* BaseCamp)
{
	RegisteredBaseCamps.Remove(BaseCamp);
}

APW_BaseCampActor* UPW_BaseCampSubsystem::FindBaseCampAtLocation(const FVector& Location) const
{
	for (APW_BaseCampActor* BaseCamp : RegisteredBaseCamps)
	{
		if (IsValid(BaseCamp) && BaseCamp->ContainsLocation(Location))
		{
			return BaseCamp;
		}
	}

	return nullptr;
}

void UPW_BaseCampSubsystem::GetBaseCampsForOwner(const FPW_BaseOwnerId& OwnerId, TArray<APW_BaseCampActor*>& OutBaseCamps) const
{
	OutBaseCamps.Reset();

	for (APW_BaseCampActor* BaseCamp : RegisteredBaseCamps)
	{
		if (IsValid(BaseCamp) && BaseCamp->GetBaseOwnerId() == OwnerId)
		{
			OutBaseCamps.Add(BaseCamp);
		}
	}
}

bool UPW_BaseCampSubsystem::CanPlaceBaseCampAtLocation(const FVector& Location, float Radius, APW_BaseCampActor* IgnoredBaseCamp) const
{
	if (Radius <= 0.0f)
	{
		return false;
	}

	const float RadiusSquared = FMath::Square(Radius);
	for (APW_BaseCampActor* BaseCamp : RegisteredBaseCamps)
	{
		if (!IsValid(BaseCamp) || BaseCamp == IgnoredBaseCamp)
		{
			continue;
		}

		const float CombinedRadius = Radius + BaseCamp->GetCampRadius();
		if (FVector::DistSquared(Location, BaseCamp->GetActorLocation()) <= FMath::Square(CombinedRadius))
		{
			return false;
		}
	}

	return RadiusSquared > 0.0f;
}

void UPW_BaseCampSubsystem::CleanupInvalidBaseCamps()
{
	RegisteredBaseCamps.RemoveAll([](const TObjectPtr<APW_BaseCampActor>& BaseCamp)
	{
		return !IsValid(BaseCamp);
	});
}
