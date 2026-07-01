// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWInventoryPanelWidget.h"

#include "Player/Core/PWPlayerCharacter.h"

void UPWInventoryPanelWidget::InitializeWithInventoryComponent(UPWPlayerInventoryLinkComponent* InInventoryComponent)
{
	if (BoundInventoryComponent == InInventoryComponent)
	{
		HandleInventoryChanged();
		return;
	}

	UnbindInventoryComponent();
	BoundInventoryComponent = InInventoryComponent;

	if (BoundInventoryComponent)
	{
		BoundInventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &UPWInventoryPanelWidget::HandleInventoryChanged);
	}

	HandleInventoryChanged();
}

void UPWInventoryPanelWidget::InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter)
{
	BoundEquipmentComponent = InPlayerCharacter ? InPlayerCharacter->GetEquipmentComponent() : nullptr;
	InitializeWithInventoryComponent(InPlayerCharacter ? InPlayerCharacter->GetInventoryLinkComponent() : nullptr);
}

TArray<FPWInventorySlotView> UPWInventoryPanelWidget::GetSlotViews() const
{
	return BoundInventoryComponent ? BoundInventoryComponent->GetSlotViews() : TArray<FPWInventorySlotView>();
}

int32 UPWInventoryPanelWidget::GetInventorySlotCount() const
{
	return BoundInventoryComponent ? BoundInventoryComponent->GetInventorySlotCount() : 0;
}

float UPWInventoryPanelWidget::GetCurrentWeight() const
{
	return BoundInventoryComponent ? BoundInventoryComponent->GetCurrentWeight() : 0.f;
}

float UPWInventoryPanelWidget::GetMaxCarryWeight() const
{
	return BoundInventoryComponent ? BoundInventoryComponent->GetMaxCarryWeight() : 0.f;
}

void UPWInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	HandleInventoryChanged();
}

void UPWInventoryPanelWidget::NativeDestruct()
{
	UnbindInventoryComponent();
	Super::NativeDestruct();
}

void UPWInventoryPanelWidget::HandleInventoryChanged()
{
	BP_OnInventoryChanged();
}

void UPWInventoryPanelWidget::UnbindInventoryComponent()
{
	if (BoundInventoryComponent)
	{
		BoundInventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UPWInventoryPanelWidget::HandleInventoryChanged);
		BoundInventoryComponent = nullptr;
	}
}
