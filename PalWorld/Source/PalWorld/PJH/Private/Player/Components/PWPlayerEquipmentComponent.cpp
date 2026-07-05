// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerEquipmentComponent.h"

#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPlayerInventoryLinkComponent.h"
#include "Player/Components/PWPlayerPrimaryActionComponent.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "Player/Data/PWItemDataAsset.h"
#include "World/PWWorldItemDropLibrary.h"

EPWToolType FPWEquipmentSlotData::GetToolType() const
{
	return ItemData && ItemData->IsEquippable()
		? ItemData->GetToolType()
		: ToolType;
}

UStaticMesh* FPWEquipmentSlotData::GetEquipmentStaticMesh() const
{
	return ItemData && ItemData->IsEquippable() && ItemData->GetEquipmentStaticMesh()
		? ItemData->GetEquipmentStaticMesh()
		: StaticMesh.Get();
}

USkeletalMesh* FPWEquipmentSlotData::GetEquipmentSkeletalMesh() const
{
	return ItemData && ItemData->IsEquippable() && ItemData->GetEquipmentSkeletalMesh()
		? ItemData->GetEquipmentSkeletalMesh()
		: SkeletalMesh.Get();
}

EPWEquipmentSlotType FPWEquipmentSlotData::GetEquipmentSlotType() const
{
	return SlotType;
}

FTransform FPWEquipmentSlotData::GetHandAttachTransform() const
{
	return ItemData && ItemData->IsEquippable() && ItemData->ShouldOverrideHandAttachTransform()
		? ItemData->GetHandAttachTransform()
		: HandAttachTransform;
}

FTransform FPWEquipmentSlotData::GetBackAttachTransform() const
{
	return ItemData && ItemData->IsEquippable() && ItemData->ShouldOverrideBackAttachTransform()
		? ItemData->GetBackAttachTransform()
		: BackAttachTransform;
}

UPWPlayerEquipmentComponent::UPWPlayerEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	EquipmentSlots.SetNum(EquipmentSlotCount);
	ApplyDefaultSlotTypes();
}

void UPWPlayerEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureSlotCount();
	RebuildVisualComponents();
	SyncSelectedToolToPrimaryActionComponent();
}

void UPWPlayerEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyVisualComponents();

	Super::EndPlay(EndPlayReason);
}

void UPWPlayerEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPWPlayerEquipmentComponent, EquipmentSlots);
	DOREPLIFETIME(UPWPlayerEquipmentComponent, SelectedSlotIndex);
}

void UPWPlayerEquipmentComponent::SelectEquipmentSlot(int32 SlotIndex)
{
	if (!IsSlotSelectable(SlotIndex))
	{
		return;
	}

	SetSelectedSlotIndex(SlotIndex);

	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (PlayerCharacter && !PlayerCharacter->HasAuthority())
	{
		ServerSelectEquipmentSlot(SlotIndex);
	}
}

void UPWPlayerEquipmentComponent::SelectNextEquipmentSlot()
{
	SelectEquipmentSlot(FindSelectableSlotByOffset(SelectedSlotIndex, 1));
}

void UPWPlayerEquipmentComponent::SelectPreviousEquipmentSlot()
{
	SelectEquipmentSlot(FindSelectableSlotByOffset(SelectedSlotIndex, -1));
}

EPWToolType UPWPlayerEquipmentComponent::GetSelectedToolType() const
{
	return IsSlotSelectable(SelectedSlotIndex)
		? EquipmentSlots[SelectedSlotIndex].GetToolType()
		: EPWToolType::Hand;
}

bool UPWPlayerEquipmentComponent::IsSlotEnabled(int32 SlotIndex) const
{
	return IsValidSlotIndex(SlotIndex) && EquipmentSlots[SlotIndex].bEnabled;
}

const FPWEquipmentSlotData& UPWPlayerEquipmentComponent::GetSlotData(int32 SlotIndex) const
{
	static const FPWEquipmentSlotData EmptySlotData;
	return IsValidSlotIndex(SlotIndex) ? EquipmentSlots[SlotIndex] : EmptySlotData;
}

EPWEquipmentSlotType UPWPlayerEquipmentComponent::GetSlotType(int32 SlotIndex) const
{
	return IsValidSlotIndex(SlotIndex) ? EquipmentSlots[SlotIndex].SlotType : EPWEquipmentSlotType::None;
}

bool UPWPlayerEquipmentComponent::IsWeaponQuickSlot(int32 SlotIndex) const
{
	return SlotIndex >= 0 && SlotIndex < WeaponSlotCount;
}

bool UPWPlayerEquipmentComponent::CanEquipItemToSlot(UPWItemDataAsset* ItemData, int32 SlotIndex) const
{
	return IsItemCompatibleWithSlot(ItemData, SlotIndex);
}

bool UPWPlayerEquipmentComponent::SetEquipmentItem(int32 SlotIndex, UPWItemDataAsset* ItemData)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerSetEquipmentItem(SlotIndex, ItemData);
		return true;
	}

	return SetEquipmentItemAuthority(SlotIndex, ItemData);
}

bool UPWPlayerEquipmentComponent::ClearEquipmentSlot(int32 SlotIndex)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerClearEquipmentSlot(SlotIndex);
		return true;
	}

	return ClearEquipmentSlotAuthority(SlotIndex);
}

bool UPWPlayerEquipmentComponent::MoveEquipmentSlot(int32 FromSlotIndex, int32 ToSlotIndex)
{
	if (FromSlotIndex == ToSlotIndex)
	{
		return false;
	}

	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerMoveEquipmentSlot(FromSlotIndex, ToSlotIndex);
		return true;
	}

	return MoveEquipmentSlotAuthority(FromSlotIndex, ToSlotIndex);
}

bool UPWPlayerEquipmentComponent::EquipFromInventorySlot(int32 InventorySlotIndex, int32 EquipmentSlotIndex)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerEquipFromInventorySlot(InventorySlotIndex, EquipmentSlotIndex);
		return true;
	}

	return EquipFromInventorySlotAuthority(InventorySlotIndex, EquipmentSlotIndex);
}

bool UPWPlayerEquipmentComponent::EquipFromInventorySlotToFirstAvailable(int32 InventorySlotIndex)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerEquipFromInventorySlotToFirstAvailable(InventorySlotIndex);
		return true;
	}

	return EquipFromInventorySlotToFirstAvailableAuthority(InventorySlotIndex);
}

bool UPWPlayerEquipmentComponent::UnequipToInventory(int32 EquipmentSlotIndex)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerUnequipToInventory(EquipmentSlotIndex);
		return true;
	}

	return UnequipToInventoryAuthority(EquipmentSlotIndex);
}

bool UPWPlayerEquipmentComponent::UnequipToInventorySlot(int32 EquipmentSlotIndex, int32 InventorySlotIndex)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerUnequipToInventorySlot(EquipmentSlotIndex, InventorySlotIndex);
		return true;
	}

	return UnequipToInventorySlotAuthority(EquipmentSlotIndex, InventorySlotIndex);
}

bool UPWPlayerEquipmentComponent::DropEquipmentSlot(int32 EquipmentSlotIndex)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerDropEquipmentSlot(EquipmentSlotIndex);
		return true;
	}

	return DropEquipmentSlotAuthority(EquipmentSlotIndex);
}

bool UPWPlayerEquipmentComponent::DestroyEquipmentSlot(int32 EquipmentSlotIndex)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		ServerDestroyEquipmentSlot(EquipmentSlotIndex);
		return true;
	}

	return ClearEquipmentSlotAuthority(EquipmentSlotIndex);
}

void UPWPlayerEquipmentComponent::ServerSelectEquipmentSlot_Implementation(int32 NewSlotIndex)
{
	if (!IsSlotSelectable(NewSlotIndex))
	{
		return;
	}

	SetSelectedSlotIndex(NewSlotIndex);
}

void UPWPlayerEquipmentComponent::ServerSetEquipmentItem_Implementation(int32 SlotIndex, UPWItemDataAsset* ItemData)
{
	SetEquipmentItemAuthority(SlotIndex, ItemData);
}

void UPWPlayerEquipmentComponent::ServerClearEquipmentSlot_Implementation(int32 SlotIndex)
{
	ClearEquipmentSlotAuthority(SlotIndex);
}

void UPWPlayerEquipmentComponent::ServerMoveEquipmentSlot_Implementation(int32 FromSlotIndex, int32 ToSlotIndex)
{
	MoveEquipmentSlotAuthority(FromSlotIndex, ToSlotIndex);
}

void UPWPlayerEquipmentComponent::ServerEquipFromInventorySlot_Implementation(int32 InventorySlotIndex, int32 EquipmentSlotIndex)
{
	EquipFromInventorySlotAuthority(InventorySlotIndex, EquipmentSlotIndex);
}

void UPWPlayerEquipmentComponent::ServerEquipFromInventorySlotToFirstAvailable_Implementation(int32 InventorySlotIndex)
{
	EquipFromInventorySlotToFirstAvailableAuthority(InventorySlotIndex);
}

void UPWPlayerEquipmentComponent::ServerUnequipToInventory_Implementation(int32 EquipmentSlotIndex)
{
	UnequipToInventoryAuthority(EquipmentSlotIndex);
}

void UPWPlayerEquipmentComponent::ServerUnequipToInventorySlot_Implementation(int32 EquipmentSlotIndex, int32 InventorySlotIndex)
{
	UnequipToInventorySlotAuthority(EquipmentSlotIndex, InventorySlotIndex);
}

void UPWPlayerEquipmentComponent::ServerDropEquipmentSlot_Implementation(int32 EquipmentSlotIndex)
{
	DropEquipmentSlotAuthority(EquipmentSlotIndex);
}

void UPWPlayerEquipmentComponent::ServerDestroyEquipmentSlot_Implementation(int32 EquipmentSlotIndex)
{
	ClearEquipmentSlotAuthority(EquipmentSlotIndex);
}

void UPWPlayerEquipmentComponent::OnRep_EquipmentSlots()
{
	EnsureSlotCount();
	RebuildVisualComponents();
	SyncSelectedToolToPrimaryActionComponent();
	OnEquipmentChanged.Broadcast();
}

void UPWPlayerEquipmentComponent::OnRep_SelectedSlotIndex()
{
	UpdateEquipmentVisuals();
	SyncSelectedToolToPrimaryActionComponent();
	OnEquipmentChanged.Broadcast();
}

APWPlayerCharacter* UPWPlayerEquipmentComponent::GetPlayerCharacter() const
{
	return Cast<APWPlayerCharacter>(GetOwner());
}

USkeletalMeshComponent* UPWPlayerEquipmentComponent::GetCharacterMesh() const
{
	const APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	return PlayerCharacter ? PlayerCharacter->GetMesh() : nullptr;
}

UPWPlayerInventoryLinkComponent* UPWPlayerEquipmentComponent::GetInventoryComponent() const
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	return PlayerCharacter ? PlayerCharacter->GetInventoryLinkComponent() : nullptr;
}

void UPWPlayerEquipmentComponent::EnsureSlotCount()
{
	if (EquipmentSlots.Num() != EquipmentSlotCount)
	{
		EquipmentSlots.SetNum(EquipmentSlotCount);
	}

	ApplyDefaultSlotTypes();

	if (!IsSlotSelectable(SelectedSlotIndex))
	{
		SelectedSlotIndex = 0;
		for (int32 SlotIndex = 0; SlotIndex < WeaponSlotCount; ++SlotIndex)
		{
			if (IsSlotSelectable(SlotIndex))
			{
				SelectedSlotIndex = SlotIndex;
				break;
			}
		}
	}
}

void UPWPlayerEquipmentComponent::ApplyDefaultSlotTypes()
{
	for (int32 SlotIndex = 0; SlotIndex < EquipmentSlots.Num(); ++SlotIndex)
	{
		FPWEquipmentSlotData& SlotData = EquipmentSlots[SlotIndex];
		SlotData.bEnabled = true;

		if (IsWeaponQuickSlot(SlotIndex))
		{
			SlotData.SlotType = EPWEquipmentSlotType::Weapon;
		}
		else if (SlotIndex == HeadSlotIndex)
		{
			SlotData.SlotType = EPWEquipmentSlotType::Head;
		}
		else if (SlotIndex == BodySlotIndex)
		{
			SlotData.SlotType = EPWEquipmentSlotType::Body;
		}
		else if (SlotIndex == ShieldSlotIndex)
		{
			SlotData.SlotType = EPWEquipmentSlotType::Shield;
		}
		else if (SlotIndex == GliderSlotIndex)
		{
			SlotData.SlotType = EPWEquipmentSlotType::Glider;
		}
		else if (SlotIndex == SphereModuleSlotIndex)
		{
			SlotData.SlotType = EPWEquipmentSlotType::SphereModule;
		}
		else if (SlotIndex >= AccessorySlotStartIndex && SlotIndex < AccessorySlotStartIndex + AccessorySlotCount)
		{
			SlotData.SlotType = EPWEquipmentSlotType::Accessory;
		}
		else if (SlotIndex >= FoodSlotStartIndex && SlotIndex < FoodSlotStartIndex + FoodSlotCount)
		{
			SlotData.SlotType = EPWEquipmentSlotType::Food;
		}
		else
		{
			SlotData.SlotType = EPWEquipmentSlotType::None;
			SlotData.bEnabled = false;
		}
	}
}

bool UPWPlayerEquipmentComponent::IsValidSlotIndex(int32 SlotIndex) const
{
	return EquipmentSlots.IsValidIndex(SlotIndex);
}

bool UPWPlayerEquipmentComponent::IsSlotSelectable(int32 SlotIndex) const
{
	// 퀵슬롯 4칸은 항상 선택 가능하다. 비어 있는 슬롯을 선택하면 맨손 상태가 된다.
	return IsValidSlotIndex(SlotIndex) && IsWeaponQuickSlot(SlotIndex);
}

bool UPWPlayerEquipmentComponent::IsItemCompatibleWithSlot(UPWItemDataAsset* ItemData, int32 SlotIndex) const
{
	if (!IsValidSlotIndex(SlotIndex))
	{
		return false;
	}

	if (!ItemData)
	{
		return true;
	}

	return ItemData->IsEquippable()
		&& ItemData->GetEquipmentSlotType() != EPWEquipmentSlotType::None
		&& ItemData->GetEquipmentSlotType() == EquipmentSlots[SlotIndex].SlotType;
}

bool UPWPlayerEquipmentComponent::CanSwapEquipmentSlots(int32 FromSlotIndex, int32 ToSlotIndex) const
{
	if (!IsValidSlotIndex(FromSlotIndex) || !IsValidSlotIndex(ToSlotIndex))
	{
		return false;
	}

	return IsItemCompatibleWithSlot(EquipmentSlots[FromSlotIndex].ItemData, ToSlotIndex)
		&& IsItemCompatibleWithSlot(EquipmentSlots[ToSlotIndex].ItemData, FromSlotIndex);
}

int32 UPWPlayerEquipmentComponent::FindSelectableSlotByOffset(int32 StartSlotIndex, int32 Offset) const
{
	if (Offset == 0)
	{
		return StartSlotIndex;
	}

	const int32 Direction = Offset > 0 ? 1 : -1;
	for (int32 Step = 1; Step <= WeaponSlotCount; ++Step)
	{
		const int32 CandidateSlot = (StartSlotIndex + Direction * Step + WeaponSlotCount) % WeaponSlotCount;
		if (IsSlotSelectable(CandidateSlot))
		{
			return CandidateSlot;
		}
	}

	return StartSlotIndex;
}

void UPWPlayerEquipmentComponent::SetSelectedSlotIndex(int32 NewSlotIndex)
{
	if (!IsSlotSelectable(NewSlotIndex) || SelectedSlotIndex == NewSlotIndex)
	{
		return;
	}

	SelectedSlotIndex = NewSlotIndex;
	UpdateEquipmentVisuals();
	SyncSelectedToolToPrimaryActionComponent();
	OnEquipmentChanged.Broadcast();
}

void UPWPlayerEquipmentComponent::SyncSelectedToolToPrimaryActionComponent() const
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		return;
	}

	if (UPWPlayerPrimaryActionComponent* PrimaryActionComponent = PlayerCharacter->GetPrimaryActionComponent())
	{
		PrimaryActionComponent->SetToolType(GetSelectedToolType());
	}
}

bool UPWPlayerEquipmentComponent::SetEquipmentItemAuthority(int32 SlotIndex, UPWItemDataAsset* ItemData)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || !IsValidSlotIndex(SlotIndex))
	{
		return false;
	}

	if (!IsItemCompatibleWithSlot(ItemData, SlotIndex))
	{
		return false;
	}

	FPWEquipmentSlotData& SlotData = EquipmentSlots[SlotIndex];
	SlotData.ItemData = ItemData;
	SlotData.ToolType = ItemData ? ItemData->GetToolType() : EPWToolType::Hand;
	SlotData.StaticMesh = nullptr;
	SlotData.SkeletalMesh = nullptr;

	NotifyEquipmentChanged();
	return true;
}

bool UPWPlayerEquipmentComponent::ClearEquipmentSlotAuthority(int32 SlotIndex)
{
	return SetEquipmentItemAuthority(SlotIndex, nullptr);
}

bool UPWPlayerEquipmentComponent::MoveEquipmentSlotAuthority(int32 FromSlotIndex, int32 ToSlotIndex)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority()
		|| FromSlotIndex == ToSlotIndex
		|| !IsValidSlotIndex(FromSlotIndex)
		|| !IsValidSlotIndex(ToSlotIndex)
		|| !CanSwapEquipmentSlots(FromSlotIndex, ToSlotIndex))
	{
		return false;
	}

	// 슬롯 타입/소켓 보정값은 위치 고정 데이터라서 아이템 관련 값만 교환한다.
	Swap(EquipmentSlots[FromSlotIndex].ItemData, EquipmentSlots[ToSlotIndex].ItemData);
	Swap(EquipmentSlots[FromSlotIndex].ToolType, EquipmentSlots[ToSlotIndex].ToolType);
	Swap(EquipmentSlots[FromSlotIndex].StaticMesh, EquipmentSlots[ToSlotIndex].StaticMesh);
	Swap(EquipmentSlots[FromSlotIndex].SkeletalMesh, EquipmentSlots[ToSlotIndex].SkeletalMesh);
	NotifyEquipmentChanged();
	return true;
}

bool UPWPlayerEquipmentComponent::EquipFromInventorySlotAuthority(int32 InventorySlotIndex, int32 EquipmentSlotIndex)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	UPWPlayerInventoryLinkComponent* InventoryComponent = GetInventoryComponent();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || !InventoryComponent || !IsValidSlotIndex(EquipmentSlotIndex))
	{
		return false;
	}

	const FPWInventoryItemStack* SourceStack = InventoryComponent->FindStackBySlot(InventorySlotIndex);
	if (!SourceStack || SourceStack->ItemId.IsNone() || SourceStack->Count <= 0)
	{
		return false;
	}

	UPWItemDataAsset* NewItemData = InventoryComponent->GetItemDefinition(SourceStack->ItemId);
	if (!IsItemCompatibleWithSlot(NewItemData, EquipmentSlotIndex))
	{
		return false;
	}

	FPWEquipmentSlotData& TargetSlot = EquipmentSlots[EquipmentSlotIndex];
	if (TargetSlot.ItemData == NewItemData)
	{
		return true;
	}

	FPWInventoryItemStack RemovedStack;
	if (!InventoryComponent->RemoveItemFromSlotAuthority(InventorySlotIndex, 1, &RemovedStack))
	{
		return false;
	}

	UPWItemDataAsset* PreviousItemData = TargetSlot.ItemData;
	if (PreviousItemData
		&& !InventoryComponent->AddItemToSlotAuthority(PreviousItemData->GetItemId(), 1, InventorySlotIndex)
		&& !InventoryComponent->AddItemAuthority(PreviousItemData->GetItemId(), 1))
	{
		InventoryComponent->AddItemAuthority(RemovedStack.ItemId, RemovedStack.Count);
		return false;
	}

	return SetEquipmentItemAuthority(EquipmentSlotIndex, NewItemData);
}

bool UPWPlayerEquipmentComponent::EquipFromInventorySlotToFirstAvailableAuthority(int32 InventorySlotIndex)
{
	UPWPlayerInventoryLinkComponent* InventoryComponent = GetInventoryComponent();
	if (!InventoryComponent)
	{
		return false;
	}

	const FPWInventoryItemStack* SourceStack = InventoryComponent->FindStackBySlot(InventorySlotIndex);
	if (!SourceStack || SourceStack->ItemId.IsNone() || SourceStack->Count <= 0)
	{
		return false;
	}

	UPWItemDataAsset* ItemData = InventoryComponent->GetItemDefinition(SourceStack->ItemId);
	int32 TargetSlotIndex = FindFirstCompatibleEquipmentSlotIndex(ItemData);
	if (TargetSlotIndex == INDEX_NONE)
	{
		return false;
	}

	return EquipFromInventorySlotAuthority(InventorySlotIndex, TargetSlotIndex);
}

bool UPWPlayerEquipmentComponent::UnequipToInventoryAuthority(int32 EquipmentSlotIndex)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	UPWPlayerInventoryLinkComponent* InventoryComponent = GetInventoryComponent();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || !InventoryComponent || !IsValidSlotIndex(EquipmentSlotIndex))
	{
		return false;
	}

	UPWItemDataAsset* ItemData = EquipmentSlots[EquipmentSlotIndex].ItemData;
	if (!ItemData)
	{
		return false;
	}

	if (!InventoryComponent->AddItemAuthority(ItemData->GetItemId(), 1))
	{
		return false;
	}

	return ClearEquipmentSlotAuthority(EquipmentSlotIndex);
}

bool UPWPlayerEquipmentComponent::UnequipToInventorySlotAuthority(int32 EquipmentSlotIndex, int32 InventorySlotIndex)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	UPWPlayerInventoryLinkComponent* InventoryComponent = GetInventoryComponent();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority()
		|| !InventoryComponent
		|| !IsValidSlotIndex(EquipmentSlotIndex))
	{
		return false;
	}

	UPWItemDataAsset* EquipmentItemData = EquipmentSlots[EquipmentSlotIndex].ItemData;
	if (!EquipmentItemData)
	{
		return false;
	}

	const FPWInventoryItemStack* TargetStack = InventoryComponent->FindStackBySlot(InventorySlotIndex);
	if (!TargetStack)
	{
		if (!InventoryComponent->AddItemToSlotAuthority(EquipmentItemData->GetItemId(), 1, InventorySlotIndex))
		{
			return false;
		}

		return ClearEquipmentSlotAuthority(EquipmentSlotIndex);
	}

	if (TargetStack->ItemId == EquipmentItemData->GetItemId())
	{
		if (!InventoryComponent->AddItemToSlotAuthority(EquipmentItemData->GetItemId(), 1, InventorySlotIndex))
		{
			return false;
		}

		return ClearEquipmentSlotAuthority(EquipmentSlotIndex);
	}

	if (TargetStack->Count != 1)
	{
		return false;
	}

	UPWItemDataAsset* InventoryItemData = InventoryComponent->GetItemDefinition(TargetStack->ItemId);
	if (!IsItemCompatibleWithSlot(InventoryItemData, EquipmentSlotIndex))
	{
		return false;
	}

	FPWInventoryItemStack RemovedStack;
	if (!InventoryComponent->RemoveItemFromSlotAuthority(InventorySlotIndex, TargetStack->Count, &RemovedStack))
	{
		return false;
	}

	if (!InventoryComponent->AddItemToSlotAuthority(EquipmentItemData->GetItemId(), 1, InventorySlotIndex))
	{
		InventoryComponent->AddItemToSlotAuthority(RemovedStack.ItemId, RemovedStack.Count, InventorySlotIndex);
		return false;
	}

	return SetEquipmentItemAuthority(EquipmentSlotIndex, InventoryItemData);
}

bool UPWPlayerEquipmentComponent::DropEquipmentSlotAuthority(int32 EquipmentSlotIndex)
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority() || !IsValidSlotIndex(EquipmentSlotIndex))
	{
		return false;
	}

	UPWItemDataAsset* ItemData = EquipmentSlots[EquipmentSlotIndex].ItemData;
	if (!UPWWorldItemDropLibrary::CanSpawnWorldItem(ItemData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWEquipment] Drop rejected. Slot=%d ItemData=%s"),
			EquipmentSlotIndex,
			*GetNameSafe(ItemData));
		return false;
	}

	const FVector Forward = PlayerCharacter->GetActorForwardVector();
	FPWWorldItemDropRequest DropRequest;
	DropRequest.ItemData = ItemData;
	DropRequest.ItemId = ItemData->GetItemId();
	DropRequest.Count = 1;
	DropRequest.SourceActor = PlayerCharacter;
	DropRequest.SourceLocation = PlayerCharacter->GetActorLocation() + Forward * 120.f + FVector(0.f, 0.f, 45.f);
	DropRequest.TargetLocation = PlayerCharacter->GetActorLocation() + Forward * 260.f;
	DropRequest.TowardTargetMinAlpha = 0.75f;
	DropRequest.TowardTargetMaxAlpha = 1.f;
	DropRequest.ScatterRadius = 35.f;
	DropRequest.bIgnoreSourceActorInGroundTrace = true;
	DropRequest.MinHorizontalImpulse = 90.f;
	DropRequest.MaxHorizontalImpulse = 160.f;
	DropRequest.MinUpwardImpulse = 140.f;
	DropRequest.MaxUpwardImpulse = 230.f;
	DropRequest.bStartAutoCollect = false;

	if (!UPWWorldItemDropLibrary::SpawnWorldItemDrop(this, DropRequest))
	{
		return false;
	}

	return ClearEquipmentSlotAuthority(EquipmentSlotIndex);
}

int32 UPWPlayerEquipmentComponent::FindFirstCompatibleEquipmentSlotIndex(UPWItemDataAsset* ItemData) const
{
	for (int32 SlotIndex = 0; SlotIndex < EquipmentSlotCount; ++SlotIndex)
	{
		if (IsValidSlotIndex(SlotIndex)
			&& !EquipmentSlots[SlotIndex].ItemData
			&& IsItemCompatibleWithSlot(ItemData, SlotIndex))
		{
			return SlotIndex;
		}
	}

	return INDEX_NONE;
}

void UPWPlayerEquipmentComponent::NotifyEquipmentChanged()
{
	EnsureSlotCount();
	RebuildVisualComponents();
	SyncSelectedToolToPrimaryActionComponent();
	OnEquipmentChanged.Broadcast();

	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->ForceNetUpdate();
	}
}

void UPWPlayerEquipmentComponent::RebuildVisualComponents()
{
	DestroyVisualComponents();

	SlotVisualComponents.SetNum(EquipmentSlotCount);
	for (int32 SlotIndex = 0; SlotIndex < EquipmentSlotCount; ++SlotIndex)
	{
		SlotVisualComponents[SlotIndex] = CreateVisualComponentForSlot(SlotIndex);
	}

	UpdateEquipmentVisuals();
}

void UPWPlayerEquipmentComponent::DestroyVisualComponents()
{
	for (TObjectPtr<USceneComponent>& VisualComponent : SlotVisualComponents)
	{
		if (VisualComponent)
		{
			VisualComponent->DestroyComponent();
			VisualComponent = nullptr;
		}
	}

	SlotVisualComponents.Reset();
}

USceneComponent* UPWPlayerEquipmentComponent::CreateVisualComponentForSlot(int32 SlotIndex)
{
	if (!IsValidSlotIndex(SlotIndex) || !EquipmentSlots[SlotIndex].HasVisualMesh())
	{
		return nullptr;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	USceneComponent* NewVisualComponent = nullptr;
	const FPWEquipmentSlotData& SlotData = EquipmentSlots[SlotIndex];
	if (USkeletalMesh* EquipmentSkeletalMesh = SlotData.GetEquipmentSkeletalMesh())
	{
		USkeletalMeshComponent* SkeletalVisualComponent = NewObject<USkeletalMeshComponent>(OwnerActor);
		SkeletalVisualComponent->SetSkeletalMesh(EquipmentSkeletalMesh);
		NewVisualComponent = SkeletalVisualComponent;
	}
	else if (UStaticMesh* EquipmentStaticMesh = SlotData.GetEquipmentStaticMesh())
	{
		UStaticMeshComponent* StaticVisualComponent = NewObject<UStaticMeshComponent>(OwnerActor);
		StaticVisualComponent->SetStaticMesh(EquipmentStaticMesh);
		NewVisualComponent = StaticVisualComponent;
	}

	if (!NewVisualComponent)
	{
		return nullptr;
	}

	NewVisualComponent->SetMobility(EComponentMobility::Movable);
	NewVisualComponent->SetHiddenInGame(true);
	NewVisualComponent->SetVisibility(false, true);

	if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(NewVisualComponent))
	{
		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimitiveComponent->SetGenerateOverlapEvents(false);
	}

	if (USkeletalMeshComponent* CharacterMesh = GetCharacterMesh())
	{
		NewVisualComponent->AttachToComponent(CharacterMesh, FAttachmentTransformRules::KeepRelativeTransform);
	}

	NewVisualComponent->RegisterComponent();
	return NewVisualComponent;
}

void UPWPlayerEquipmentComponent::UpdateEquipmentVisuals()
{
	if (SlotVisualComponents.Num() != EquipmentSlotCount)
	{
		return;
	}

	for (int32 SlotIndex = 0; SlotIndex < EquipmentSlotCount; ++SlotIndex)
	{
		if (!SlotVisualComponents[SlotIndex] || !IsSlotEnabled(SlotIndex))
		{
			HideSlotVisual(SlotIndex);
			continue;
		}

		if (SlotIndex == SelectedSlotIndex)
		{
			AttachSlotVisual(SlotIndex, HandSocketName, EquipmentSlots[SlotIndex].GetHandAttachTransform());
			continue;
		}

		if (ShouldShowSlotOnBack(SlotIndex))
		{
			AttachSlotVisual(SlotIndex, GetBackSocketNameForSlot(SlotIndex), EquipmentSlots[SlotIndex].GetBackAttachTransform());
			continue;
		}

		HideSlotVisual(SlotIndex);
	}
}

void UPWPlayerEquipmentComponent::AttachSlotVisual(int32 SlotIndex, FName SocketName, const FTransform& RelativeTransform)
{
	if (!SlotVisualComponents.IsValidIndex(SlotIndex) || !SlotVisualComponents[SlotIndex])
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = GetCharacterMesh();
	if (!CharacterMesh)
	{
		HideSlotVisual(SlotIndex);
		return;
	}

	// 손/등 전환은 스폰/파괴 대신 소켓 재부착만 한다.
	USceneComponent* VisualComponent = SlotVisualComponents[SlotIndex];
	VisualComponent->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	VisualComponent->SetRelativeTransform(RelativeTransform);
	VisualComponent->SetHiddenInGame(false);
	VisualComponent->SetVisibility(true, true);
}

void UPWPlayerEquipmentComponent::HideSlotVisual(int32 SlotIndex)
{
	if (!SlotVisualComponents.IsValidIndex(SlotIndex) || !SlotVisualComponents[SlotIndex])
	{
		return;
	}

	SlotVisualComponents[SlotIndex]->SetHiddenInGame(true);
	SlotVisualComponents[SlotIndex]->SetVisibility(false, true);
}

FName UPWPlayerEquipmentComponent::GetBackSocketNameForSlot(int32 SlotIndex) const
{
	return SlotIndex == 0 ? BackLeftSocketName : BackRightSocketName;
}

bool UPWPlayerEquipmentComponent::ShouldShowSlotOnBack(int32 SlotIndex) const
{
	// 팰월드식으로 1번/2번 슬롯만 등에 노출하고, 3번/4번은 선택 중일 때만 손에 보인다.
	return SlotIndex == 0 || SlotIndex == 1;
}
