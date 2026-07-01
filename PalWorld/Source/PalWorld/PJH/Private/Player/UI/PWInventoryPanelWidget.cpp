// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWInventoryPanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Player/Data/PWItemDataAsset.h"
#include "Styling/SlateColor.h"

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

TSharedRef<SWidget> UPWInventoryPanelWidget::RebuildWidget()
{
	BuildDefaultLayout();
	return Super::RebuildWidget();
}

void UPWInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshDefaultLayout();
}

void UPWInventoryPanelWidget::NativeDestruct()
{
	UnbindInventoryComponent();
	Super::NativeDestruct();
}

void UPWInventoryPanelWidget::HandleInventoryChanged()
{
	RefreshDefaultLayout();
	BP_OnInventoryChanged();
}

void UPWInventoryPanelWidget::BuildDefaultLayout()
{
	if (!bUseDefaultRuntimeLayout)
	{
		return;
	}

	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}

	if (!WidgetTree)
	{
		return;
	}

	bUsingDefaultLayout = true;

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("InventoryRoot"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InventoryBackdrop"));
	Backdrop->SetBrushColor(FLinearColor(0.01f, 0.035f, 0.04f, 0.92f));

	UCanvasPanelSlot* BackdropSlot = RootCanvas->AddChildToCanvas(Backdrop);
	BackdropSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	BackdropSlot->SetOffsets(FMargin(20.f));

	UVerticalBox* MainVertical = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InventoryMainVertical"));
	Backdrop->SetContent(MainVertical);

	UHorizontalBox* TabBar = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("InventoryTabBar"));
	UVerticalBoxSlot* TabBarSlot = MainVertical->AddChildToVerticalBox(TabBar);
	TabBarSlot->SetPadding(FMargin(16.f, 12.f, 16.f, 10.f));
	TabBarSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

	const TArray<FText> Tabs = {
		FText::FromString(TEXT("인벤토리")),
		FText::FromString(TEXT("보유 팰")),
		FText::FromString(TEXT("기술")),
		FText::FromString(TEXT("임무")),
		FText::FromString(TEXT("팰 도감")),
		FText::FromString(TEXT("길드")),
		FText::FromString(TEXT("옵션"))
	};

	for (int32 TabIndex = 0; TabIndex < Tabs.Num(); ++TabIndex)
	{
		UBorder* TabBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		TabBorder->SetBrushColor(TabIndex == 0 ? FLinearColor(0.08f, 0.78f, 1.f, 0.85f) : FLinearColor(0.03f, 0.08f, 0.1f, 0.8f));

		UTextBlock* TabText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		TabText->SetText(Tabs[TabIndex]);
		TabText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		TabText->SetJustification(ETextJustify::Center);
		TabBorder->SetContent(TabText);

		UHorizontalBoxSlot* TabSlot = TabBar->AddChildToHorizontalBox(TabBorder);
		TabSlot->SetPadding(FMargin(4.f, 0.f));
		TabSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("InventoryBody"));
	UVerticalBoxSlot* BodySlot = MainVertical->AddChildToVerticalBox(Body);
	BodySlot->SetPadding(FMargin(20.f, 8.f, 20.f, 20.f));
	BodySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	UBorder* LeftPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InventoryGridPanel"));
	LeftPanel->SetBrushColor(FLinearColor(0.02f, 0.05f, 0.055f, 0.82f));
	UHorizontalBoxSlot* LeftSlot = Body->AddChildToHorizontalBox(LeftPanel);
	LeftSlot->SetPadding(FMargin(0.f, 0.f, 20.f, 0.f));
	LeftSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	UVerticalBox* LeftVertical = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InventoryLeftVertical"));
	LeftPanel->SetContent(LeftVertical);

	DefaultTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("InventoryTitle"));
	DefaultTitleText->SetText(FText::FromString(TEXT("인벤토리")));
	DefaultTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.82f, 1.f, 1.f, 1.f)));
	DefaultTitleText->SetJustification(ETextJustify::Center);
	UVerticalBoxSlot* TitleSlot = LeftVertical->AddChildToVerticalBox(DefaultTitleText);
	TitleSlot->SetPadding(FMargin(12.f, 10.f, 12.f, 8.f));
	TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

	DefaultInventoryGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("InventoryGrid"));
	UVerticalBoxSlot* GridSlot = LeftVertical->AddChildToVerticalBox(DefaultInventoryGrid);
	GridSlot->SetPadding(FMargin(12.f));
	GridSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

	DefaultWeightText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("InventoryWeight"));
	DefaultWeightText->SetColorAndOpacity(FSlateColor(FLinearColor(0.82f, 1.f, 1.f, 1.f)));
	UVerticalBoxSlot* WeightSlot = LeftVertical->AddChildToVerticalBox(DefaultWeightText);
	WeightSlot->SetPadding(FMargin(12.f, 8.f, 12.f, 12.f));
	WeightSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

	UBorder* CenterPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InventoryCharacterPanel"));
	CenterPanel->SetBrushColor(FLinearColor(0.015f, 0.035f, 0.04f, 0.45f));
	UTextBlock* CenterText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	CenterText->SetText(FText::FromString(TEXT("캐릭터 / 장비 영역\n다음 단계에서 장비 슬롯과 프리뷰 연결")));
	CenterText->SetColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.95f, 1.f, 1.f)));
	CenterText->SetJustification(ETextJustify::Center);
	CenterPanel->SetContent(CenterText);
	UHorizontalBoxSlot* CenterSlot = Body->AddChildToHorizontalBox(CenterPanel);
	CenterSlot->SetPadding(FMargin(0.f, 0.f, 20.f, 0.f));
	CenterSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	UBorder* RightPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InventoryStatPanel"));
	RightPanel->SetBrushColor(FLinearColor(0.02f, 0.05f, 0.055f, 0.82f));
	UTextBlock* RightText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	RightText->SetText(FText::FromString(TEXT("PLAYER LEVEL\nHP / 스태미나 / 스탯\n다음 단계에서 StatComponent 연결")));
	RightText->SetColorAndOpacity(FSlateColor(FLinearColor(0.82f, 1.f, 1.f, 1.f)));
	RightText->SetJustification(ETextJustify::Center);
	RightPanel->SetContent(RightText);
	UHorizontalBoxSlot* RightSlot = Body->AddChildToHorizontalBox(RightPanel);
	RightSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
}

void UPWInventoryPanelWidget::RefreshDefaultLayout()
{
	if (!bUsingDefaultLayout)
	{
		return;
	}

	if (DefaultInventoryGrid)
	{
		DefaultInventoryGrid->ClearChildren();

		const TArray<FPWInventorySlotView> SlotViews = GetSlotViews();
		const int32 SlotCount = SlotViews.Num() > 0 ? SlotViews.Num() : 40;
		for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
		{
			FPWInventorySlotView SlotView;
			if (SlotViews.IsValidIndex(SlotIndex))
			{
				SlotView = SlotViews[SlotIndex];
			}
			else
			{
				SlotView.SlotIndex = SlotIndex;
			}

			UWidget* SlotWidget = CreateDefaultSlotWidget(SlotView);
			UUniformGridSlot* GridChildSlot = DefaultInventoryGrid->AddChildToUniformGrid(SlotWidget, SlotIndex / 8, SlotIndex % 8);
			GridChildSlot->SetHorizontalAlignment(HAlign_Fill);
			GridChildSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}

	if (DefaultWeightText)
	{
		DefaultWeightText->SetText(FText::FromString(FString::Printf(
			TEXT("소지 중량   %.1f / %.1f"),
			GetCurrentWeight(),
			GetMaxCarryWeight()
		)));
	}
}

UWidget* UPWInventoryPanelWidget::CreateDefaultSlotWidget(const FPWInventorySlotView& SlotView)
{
	USizeBox* SlotSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SlotSizeBox->SetWidthOverride(66.f);
	SlotSizeBox->SetHeightOverride(66.f);

	UBorder* SlotBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	SlotBorder->SetBrushColor(SlotView.bOccupied ? FLinearColor(0.02f, 0.18f, 0.2f, 0.95f) : FLinearColor(0.04f, 0.06f, 0.065f, 0.82f));
	SlotSizeBox->SetContent(SlotBorder);

	UTextBlock* SlotText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	SlotText->SetJustification(ETextJustify::Center);
	SlotText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

	if (SlotView.bOccupied)
	{
		const FString ItemName = SlotView.ItemData
			? SlotView.ItemData->GetDisplayName().ToString()
			: SlotView.ItemId.ToString();
		SlotText->SetText(FText::FromString(FString::Printf(TEXT("%s\nx%d"), *ItemName, SlotView.Count)));
	}
	else
	{
		SlotText->SetText(FText::FromString(TEXT("-")));
	}

	SlotBorder->SetContent(SlotText);
	return SlotSizeBox;
}

void UPWInventoryPanelWidget::UnbindInventoryComponent()
{
	if (BoundInventoryComponent)
	{
		BoundInventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UPWInventoryPanelWidget::HandleInventoryChanged);
		BoundInventoryComponent = nullptr;
	}
}
