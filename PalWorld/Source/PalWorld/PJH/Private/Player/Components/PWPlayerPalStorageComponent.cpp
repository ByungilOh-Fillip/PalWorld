// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerPalStorageComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "PWPalBase.h"
#include "StatusComponent.h"

UPWPlayerPalStorageComponent::UPWPlayerPalStorageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPWPlayerPalStorageComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPWPlayerPalStorageComponent, CapturedPals);
	DOREPLIFETIME(UPWPlayerPalStorageComponent, ActiveSummonedPal);
	DOREPLIFETIME(UPWPlayerPalStorageComponent, ActiveSummonedPartySlotIndex);
	DOREPLIFETIME(UPWPlayerPalStorageComponent, SelectedPartySlotIndex);
}

bool UPWPlayerPalStorageComponent::RegisterCapturedPal(APWPalBase* CapturedPal)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || !IsValid(CapturedPal))
	{
		return false;
	}

	FPWCapturedPalRecord NewRecord = MakeCapturedPalRecord(CapturedPal);
	if (!NewRecord.PalClass)
	{
		return false;
	}

	if (CapturedPals.Num() < MaxPartySlots)
	{
		NewRecord.PartySlotIndex = CapturedPals.Num();
	}

	CapturedPals.Add(NewRecord);
	if (CapturedPals.Num() == 1 && NewRecord.PartySlotIndex != INDEX_NONE)
	{
		SelectedPartySlotIndex = NewRecord.PartySlotIndex;
	}
	OnCapturedPalsChanged.Broadcast();

	UE_LOG(LogTemp, Display, TEXT("[PWPalStorage] Captured pal registered. Owner=%s PalId=%s Name=%s PartySlot=%d Total=%d"),
		*GetNameSafe(Owner),
		*NewRecord.PalId.ToString(),
		*NewRecord.DisplayName,
		NewRecord.PartySlotIndex,
		CapturedPals.Num());

	return true;
}

bool UPWPlayerPalStorageComponent::ToggleSummonPartyPal(int32 PartySlotIndex)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	if (!Owner->HasAuthority())
	{
		ServerToggleSummonPartyPal(PartySlotIndex);
		return true;
	}

	FPWCapturedPalRecord* Record = FindPartyRecord(PartySlotIndex);
	if (!Record || !Record->PalClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWPalStorage] Toggle summon failed. PartySlot=%d Reason=NoRecord"), PartySlotIndex);
		return false;
	}

	if (IsValid(ActiveSummonedPal))
	{
		if (ActiveSummonedPartySlotIndex == PartySlotIndex)
		{
			return RecallActivePalAuthority();
		}

		// 선택된 팰이 현재 소환 팰과 다를 때만 E 입력에서 교체를 실행한다.
		RecallActivePalAuthority();
	}

	SelectedPartySlotIndex = PartySlotIndex;
	return SummonPartyPalAuthority(PartySlotIndex);
}

bool UPWPlayerPalStorageComponent::ToggleSummonSelectedPartyPal()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	if (!Owner->HasAuthority())
	{
		ServerToggleSummonSelectedPartyPal();
		return true;
	}

	return ToggleSummonPartyPal(SelectedPartySlotIndex);
}

bool UPWPlayerPalStorageComponent::SelectPreviousPartyPal()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	if (!Owner->HasAuthority())
	{
		ServerSelectPartyPal(-1);
		return true;
	}

	return SelectPartyPalAuthority(-1);
}

bool UPWPlayerPalStorageComponent::SelectNextPartyPal()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	if (!Owner->HasAuthority())
	{
		ServerSelectPartyPal(1);
		return true;
	}

	return SelectPartyPalAuthority(1);
}

void UPWPlayerPalStorageComponent::OnRep_CapturedPals()
{
	OnCapturedPalsChanged.Broadcast();
}

void UPWPlayerPalStorageComponent::OnRep_SelectedPartySlotIndex()
{
	OnCapturedPalsChanged.Broadcast();
}

void UPWPlayerPalStorageComponent::ServerToggleSummonPartyPal_Implementation(int32 PartySlotIndex)
{
	ToggleSummonPartyPal(PartySlotIndex);
}

void UPWPlayerPalStorageComponent::ServerToggleSummonSelectedPartyPal_Implementation()
{
	ToggleSummonSelectedPartyPal();
}

void UPWPlayerPalStorageComponent::ServerSelectPartyPal_Implementation(int32 Direction)
{
	SelectPartyPalAuthority(Direction);
}

FPWCapturedPalRecord UPWPlayerPalStorageComponent::MakeCapturedPalRecord(APWPalBase* CapturedPal) const
{
	FPWCapturedPalRecord Record;
	if (!CapturedPal)
	{
		return Record;
	}

	Record.InstanceId = FGuid::NewGuid();
	Record.PalId = ResolvePalId(CapturedPal);
	Record.DisplayName = ResolveDisplayName(CapturedPal);
	Record.PalClass = CapturedPal->GetClass();
	Record.CapturedWorldTransform = CapturedPal->GetActorTransform();

	if (const UStatusComponent* StatusComponent = CapturedPal->FindComponentByClass<UStatusComponent>())
	{
		Record.Level = FMath::Max(1, StatusComponent->Level);
		Record.CurrentHP = FMath::Max(0.f, StatusComponent->CurrentHP);
		Record.MaxHP = FMath::Max(0.f, StatusComponent->MaxHP);
	}

	return Record;
}

FName UPWPlayerPalStorageComponent::ResolvePalId(APWPalBase* CapturedPal) const
{
	if (!CapturedPal || !CapturedPal->GetClass())
	{
		return NAME_None;
	}

	FString ClassName = CapturedPal->GetClass()->GetName();
	ClassName.RemoveFromEnd(TEXT("_C"));
	return FName(*ClassName);
}

FString UPWPlayerPalStorageComponent::ResolveDisplayName(APWPalBase* CapturedPal) const
{
	if (!CapturedPal || !CapturedPal->GetClass())
	{
		return FString();
	}

	FString DisplayName = CapturedPal->GetClass()->GetName();
	DisplayName.RemoveFromEnd(TEXT("_C"));
	DisplayName.RemoveFromStart(TEXT("BP_"));
	DisplayName.ReplaceInline(TEXT("_"), TEXT(" "));
	return DisplayName;
}

bool UPWPlayerPalStorageComponent::SelectPartyPalAuthority(int32 Direction)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || Direction == 0)
	{
		return false;
	}

	const int32 NextSlotIndex = FindNextPartySlotIndex(SelectedPartySlotIndex, Direction);
	if (NextSlotIndex == INDEX_NONE || NextSlotIndex == SelectedPartySlotIndex)
	{
		return false;
	}

	SelectedPartySlotIndex = NextSlotIndex;
	OnCapturedPalsChanged.Broadcast();

	UE_LOG(LogTemp, Display, TEXT("[PWPalStorage] Selected party pal loaded. Owner=%s SelectedSlot=%d ActiveSlot=%d"),
		*GetNameSafe(Owner),
		SelectedPartySlotIndex,
		ActiveSummonedPartySlotIndex);

	return true;
}

bool UPWPlayerPalStorageComponent::SummonPartyPalAuthority(int32 PartySlotIndex)
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !Owner->HasAuthority() || !World)
	{
		return false;
	}

	FPWCapturedPalRecord* Record = FindPartyRecord(PartySlotIndex);
	if (!Record || !Record->PalClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWPalStorage] Summon failed. PartySlot=%d Reason=NoRecord"), PartySlotIndex);
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Owner;
	SpawnParameters.Instigator = Cast<APawn>(Owner);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APWPalBase* SummonedPal = World->SpawnActor<APWPalBase>(Record->PalClass, ResolveSummonTransform(), SpawnParameters);
	if (!SummonedPal)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWPalStorage] Summon failed. PartySlot=%d PalId=%s Reason=SpawnFailed"),
			PartySlotIndex,
			*Record->PalId.ToString());
		return false;
	}

	ApplyRecordToSummonedPal(SummonedPal, *Record);
	ActiveSummonedPal = SummonedPal;
	ActiveSummonedPartySlotIndex = PartySlotIndex;
	SelectedPartySlotIndex = PartySlotIndex;

	UE_LOG(LogTemp, Display, TEXT("[PWPalStorage] Party pal summoned. Owner=%s PartySlot=%d Pal=%s Location=%s"),
		*GetNameSafe(Owner),
		PartySlotIndex,
		*GetNameSafe(SummonedPal),
		*SummonedPal->GetActorLocation().ToString());

	return true;
}

bool UPWPlayerPalStorageComponent::RecallActivePalAuthority()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || !IsValid(ActiveSummonedPal))
	{
		ActiveSummonedPal = nullptr;
		ActiveSummonedPartySlotIndex = INDEX_NONE;
		return false;
	}

	SaveActivePalStateToRecord();

	UE_LOG(LogTemp, Display, TEXT("[PWPalStorage] Party pal recalled. Owner=%s PartySlot=%d Pal=%s"),
		*GetNameSafe(Owner),
		ActiveSummonedPartySlotIndex,
		*GetNameSafe(ActiveSummonedPal));

	ActiveSummonedPal->Destroy();
	ActiveSummonedPal = nullptr;
	ActiveSummonedPartySlotIndex = INDEX_NONE;
	OnCapturedPalsChanged.Broadcast();
	return true;
}

FPWCapturedPalRecord* UPWPlayerPalStorageComponent::FindPartyRecord(int32 PartySlotIndex)
{
	for (FPWCapturedPalRecord& CapturedPal : CapturedPals)
	{
		if (CapturedPal.PartySlotIndex == PartySlotIndex)
		{
			return &CapturedPal;
		}
	}

	return nullptr;
}

const FPWCapturedPalRecord* UPWPlayerPalStorageComponent::FindPartyRecord(int32 PartySlotIndex) const
{
	for (const FPWCapturedPalRecord& CapturedPal : CapturedPals)
	{
		if (CapturedPal.PartySlotIndex == PartySlotIndex)
		{
			return &CapturedPal;
		}
	}

	return nullptr;
}

bool UPWPlayerPalStorageComponent::HasPartyRecord(int32 PartySlotIndex) const
{
	return FindPartyRecord(PartySlotIndex) != nullptr;
}

int32 UPWPlayerPalStorageComponent::FindNextPartySlotIndex(int32 StartSlotIndex, int32 Direction) const
{
	if (CapturedPals.IsEmpty() || MaxPartySlots <= 0)
	{
		return INDEX_NONE;
	}

	const int32 Step = Direction > 0 ? 1 : -1;
	int32 CurrentSlotIndex = FMath::Clamp(StartSlotIndex, 0, FMath::Max(MaxPartySlots - 1, 0));
	for (int32 Attempt = 0; Attempt < MaxPartySlots; ++Attempt)
	{
		CurrentSlotIndex = (CurrentSlotIndex + Step + MaxPartySlots) % MaxPartySlots;
		if (HasPartyRecord(CurrentSlotIndex))
		{
			return CurrentSlotIndex;
		}
	}

	return HasPartyRecord(StartSlotIndex) ? StartSlotIndex : INDEX_NONE;
}

FTransform UPWPlayerPalStorageComponent::ResolveSummonTransform() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FTransform::Identity;
	}

	const FVector Forward = Owner->GetActorForwardVector().GetSafeNormal2D();
	const FVector Right = Owner->GetActorRightVector().GetSafeNormal2D();
	FVector SpawnLocation = Owner->GetActorLocation() + Forward * 260.f + Right * 90.f;
	SpawnLocation.Z += 60.f;

	if (const UWorld* World = GetWorld())
	{
		FHitResult GroundHit;
		const FVector TraceStart = SpawnLocation + FVector(0.f, 0.f, 300.f);
		const FVector TraceEnd = SpawnLocation - FVector(0.f, 0.f, 700.f);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PWPalSummonGroundTrace), false, Owner);
		if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			SpawnLocation = GroundHit.ImpactPoint + FVector(0.f, 0.f, 8.f);
		}
	}

	return FTransform(Owner->GetActorRotation(), SpawnLocation);
}

void UPWPlayerPalStorageComponent::ApplyRecordToSummonedPal(APWPalBase* SummonedPal, const FPWCapturedPalRecord& Record) const
{
	if (!SummonedPal)
	{
		return;
	}

	SummonedPal->SetOwner(GetOwner());
	SummonedPal->SetReplicates(true);
	SummonedPal->SetReplicateMovement(true);
	SummonedPal->SetPlayerOwnedPal(true);

	if (UStatusComponent* StatusComponent = SummonedPal->FindComponentByClass<UStatusComponent>())
	{
		StatusComponent->Level = FMath::Max(1, Record.Level);
		if (Record.MaxHP > 0.f)
		{
			StatusComponent->MaxHP = Record.MaxHP;
		}
		StatusComponent->CurrentHP = Record.CurrentHP > 0.f
			? FMath::Min(Record.CurrentHP, StatusComponent->MaxHP)
			: StatusComponent->MaxHP;
	}
}

void UPWPlayerPalStorageComponent::SaveActivePalStateToRecord()
{
	FPWCapturedPalRecord* Record = FindPartyRecord(ActiveSummonedPartySlotIndex);
	if (!Record || !IsValid(ActiveSummonedPal))
	{
		return;
	}

	Record->CapturedWorldTransform = ActiveSummonedPal->GetActorTransform();

	if (const UStatusComponent* StatusComponent = ActiveSummonedPal->FindComponentByClass<UStatusComponent>())
	{
		Record->Level = FMath::Max(1, StatusComponent->Level);
		Record->CurrentHP = FMath::Max(0.f, StatusComponent->CurrentHP);
		Record->MaxHP = FMath::Max(0.f, StatusComponent->MaxHP);
	}
}
