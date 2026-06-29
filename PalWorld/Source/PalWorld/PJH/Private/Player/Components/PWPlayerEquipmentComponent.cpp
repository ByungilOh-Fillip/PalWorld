// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/PWPlayerEquipmentComponent.h"

#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/PWPlayerGatherComponent.h"
#include "Player/Core/PWPlayerCharacter.h"

UPWPlayerEquipmentComponent::UPWPlayerEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	EquipmentSlots.SetNum(EquipmentSlotCount);
	EquipmentSlots[0].bEnabled = true;
}

void UPWPlayerEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureSlotCount();
	RebuildVisualComponents();
	SyncSelectedToolToGatherComponent();
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
		? EquipmentSlots[SelectedSlotIndex].ToolType
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

void UPWPlayerEquipmentComponent::ServerSelectEquipmentSlot_Implementation(int32 NewSlotIndex)
{
	if (!IsSlotSelectable(NewSlotIndex))
	{
		return;
	}

	SetSelectedSlotIndex(NewSlotIndex);
}

void UPWPlayerEquipmentComponent::OnRep_EquipmentSlots()
{
	EnsureSlotCount();
	RebuildVisualComponents();
	SyncSelectedToolToGatherComponent();
}

void UPWPlayerEquipmentComponent::OnRep_SelectedSlotIndex()
{
	UpdateEquipmentVisuals();
	SyncSelectedToolToGatherComponent();
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

void UPWPlayerEquipmentComponent::EnsureSlotCount()
{
	if (EquipmentSlots.Num() != EquipmentSlotCount)
	{
		EquipmentSlots.SetNum(EquipmentSlotCount);
	}

	if (!IsSlotSelectable(SelectedSlotIndex))
	{
		SelectedSlotIndex = 0;
		for (int32 SlotIndex = 0; SlotIndex < EquipmentSlotCount; ++SlotIndex)
		{
			if (IsSlotSelectable(SlotIndex))
			{
				SelectedSlotIndex = SlotIndex;
				break;
			}
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
	return IsValidSlotIndex(SlotIndex);
}

int32 UPWPlayerEquipmentComponent::FindSelectableSlotByOffset(int32 StartSlotIndex, int32 Offset) const
{
	if (Offset == 0)
	{
		return StartSlotIndex;
	}

	const int32 Direction = Offset > 0 ? 1 : -1;
	for (int32 Step = 1; Step <= EquipmentSlotCount; ++Step)
	{
		const int32 CandidateSlot = (StartSlotIndex + Direction * Step + EquipmentSlotCount) % EquipmentSlotCount;
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
	SyncSelectedToolToGatherComponent();
}

void UPWPlayerEquipmentComponent::SyncSelectedToolToGatherComponent() const
{
	APWPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority())
	{
		return;
	}

	if (UPWPlayerGatherComponent* GatherComponent = PlayerCharacter->GetGatherComponent())
	{
		GatherComponent->RequestEquipTool(GetSelectedToolType());
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
	if (SlotData.SkeletalMesh)
	{
		USkeletalMeshComponent* SkeletalVisualComponent = NewObject<USkeletalMeshComponent>(OwnerActor);
		SkeletalVisualComponent->SetSkeletalMesh(SlotData.SkeletalMesh);
		NewVisualComponent = SkeletalVisualComponent;
	}
	else if (SlotData.StaticMesh)
	{
		UStaticMeshComponent* StaticVisualComponent = NewObject<UStaticMeshComponent>(OwnerActor);
		StaticVisualComponent->SetStaticMesh(SlotData.StaticMesh);
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
			AttachSlotVisual(SlotIndex, HandSocketName, EquipmentSlots[SlotIndex].HandAttachTransform);
			continue;
		}

		if (ShouldShowSlotOnBack(SlotIndex))
		{
			AttachSlotVisual(SlotIndex, GetBackSocketNameForSlot(SlotIndex), EquipmentSlots[SlotIndex].BackAttachTransform);
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
