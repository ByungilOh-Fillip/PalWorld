// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/PWCaptureAimWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Player/Components/PWPlayerCaptureComponent.h"
#include "Player/Core/PWPlayerCharacter.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	const TCHAR* CaptureTexturePath(int32 Index)
	{
		switch (Index)
		{
		case 1:
			return TEXT("/Game/PJH/UI/Widgets/CT_IMG/ChatGPT_Image_2026년_7월_7일_오후_05_57_50__1_-Photoroom.ChatGPT_Image_2026년_7월_7일_오후_05_57_50__1_-Photoroom");
		case 2:
			return TEXT("/Game/PJH/UI/Widgets/CT_IMG/ChatGPT_Image_2026년_7월_7일_오후_05_57_50__2_-Photoroom.ChatGPT_Image_2026년_7월_7일_오후_05_57_50__2_-Photoroom");
		case 3:
			return TEXT("/Game/PJH/UI/Widgets/CT_IMG/ChatGPT_Image_2026년_7월_7일_오후_05_57_50__3_-Photoroom.ChatGPT_Image_2026년_7월_7일_오후_05_57_50__3_-Photoroom");
		case 4:
			return TEXT("/Game/PJH/UI/Widgets/CT_IMG/ChatGPT_Image_2026년_7월_7일_오후_05_57_50__4_-Photoroom.ChatGPT_Image_2026년_7월_7일_오후_05_57_50__4_-Photoroom");
		case 5:
			return TEXT("/Game/PJH/UI/Widgets/CT_IMG/ChatGPT_Image_2026년_7월_7일_오후_05_57_50__5_-Photoroom.ChatGPT_Image_2026년_7월_7일_오후_05_57_50__5_-Photoroom");
		case 6:
			return TEXT("/Game/PJH/UI/Widgets/CT_IMG/ChatGPT_Image_2026년_7월_7일_오후_05_57_50__6_-Photoroom.ChatGPT_Image_2026년_7월_7일_오후_05_57_50__6_-Photoroom");
		case 7:
			return TEXT("/Game/PJH/UI/Widgets/CT_IMG/ChatGPT_Image_2026년_7월_7일_오후_05_57_50__7_-Photoroom.ChatGPT_Image_2026년_7월_7일_오후_05_57_50__7_-Photoroom");
		case 8:
			return TEXT("/Game/PJH/UI/Widgets/CT_IMG/ChatGPT_Image_2026년_7월_7일_오후_05_57_50__8_-Photoroom.ChatGPT_Image_2026년_7월_7일_오후_05_57_50__8_-Photoroom");
		case 9:
			return TEXT("/Game/PJH/UI/Widgets/CT_IMG/ChatGPT_Image_2026년_7월_7일_오후_05_57_50__9_-Photoroom.ChatGPT_Image_2026년_7월_7일_오후_05_57_50__9_-Photoroom");
		case 10:
			return TEXT("/Game/PJH/UI/Widgets/CT_IMG/ChatGPT_Image_2026년_7월_7일_오후_05_57_50__10_-Photoroom.ChatGPT_Image_2026년_7월_7일_오후_05_57_50__10_-Photoroom");
		default:
			return TEXT("");
		}
	}

	FLinearColor CaptureTextColor()
	{
		return FLinearColor(0.82f, 1.f, 1.f, 1.f);
	}
}

void UPWCaptureAimWidget::InitializeWithPlayerCharacter(APWPlayerCharacter* InPlayerCharacter)
{
	BoundPlayerCharacter = InPlayerCharacter;
	BindCaptureComponent(InPlayerCharacter ? InPlayerCharacter->GetCaptureComponent() : nullptr);
	RefreshCaptureAim();
}

void UPWCaptureAimWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildDefaultLayout();
	RefreshCaptureAim();
}

void UPWCaptureAimWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	BuildDefaultLayout();
	if (IsDesignTime())
	{
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		for (UWidget* TargetOnlyWidget : TargetOnlyWidgets)
		{
			if (TargetOnlyWidget)
			{
				TargetOnlyWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
		}

		if (Text_TargetName)
		{
			Text_TargetName->SetText(NSLOCTEXT("PWCaptureAim", "DesignTargetName", "꼬꼬닭"));
		}

		if (Text_Chance)
		{
			Text_Chance->SetText(NSLOCTEXT("PWCaptureAim", "DesignChance", "34%"));
		}

		if (Text_SphereCount)
		{
			Text_SphereCount->SetText(NSLOCTEXT("PWCaptureAim", "DesignSphereCount", "CAPTURED  6/12"));
		}
	}
}

void UPWCaptureAimWidget::NativeDestruct()
{
	UnbindCaptureComponent();
	Super::NativeDestruct();
}

void UPWCaptureAimWidget::HandleCaptureAimChanged()
{
	RefreshCaptureAim();
}

void UPWCaptureAimWidget::BuildDefaultLayout()
{
	if (bLayoutBuilt || !WidgetTree)
	{
		return;
	}

	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CaptureAimRoot"));
	WidgetTree->RootWidget = RootCanvas;

	AddCaptureImage(TEXT("Image_OuterRing"), LoadCaptureTexture(CaptureTexturePath(3)), FVector2D(0.f, 0.f), FVector2D(460.f, 460.f), 0.82f);
	AddCaptureImage(TEXT("Image_LeftArc"), LoadCaptureTexture(CaptureTexturePath(8)), FVector2D(-310.f, 0.f), FVector2D(120.f, 500.f), 0.78f);
	AddCaptureImage(TEXT("Image_RightArc"), LoadCaptureTexture(CaptureTexturePath(9)), FVector2D(310.f, -10.f), FVector2D(260.f, 310.f), 0.78f);
	AddCaptureImage(TEXT("Image_BottomArc"), LoadCaptureTexture(CaptureTexturePath(10)), FVector2D(75.f, 230.f), FVector2D(280.f, 190.f), 0.75f);
	AddCaptureImage(TEXT("Image_UpGuide"), LoadCaptureTexture(CaptureTexturePath(7)), FVector2D(0.f, -300.f), FVector2D(68.f, 220.f), 0.82f);
	AddCaptureImage(TEXT("Image_DownGuide"), LoadCaptureTexture(CaptureTexturePath(6)), FVector2D(0.f, 300.f), FVector2D(90.f, 280.f), 0.82f);
	AddCaptureImage(TEXT("Image_LeftTicks"), LoadCaptureTexture(CaptureTexturePath(4)), FVector2D(-210.f, 0.f), FVector2D(210.f, 70.f), 0.78f);
	AddCaptureImage(TEXT("Image_RightTicks"), LoadCaptureTexture(CaptureTexturePath(5)), FVector2D(210.f, 0.f), FVector2D(210.f, 70.f), 0.78f);
	AddCaptureImage(TEXT("Image_CenterReticle"), LoadCaptureTexture(CaptureTexturePath(2)), FVector2D(0.f, 0.f), FVector2D(138.f, 138.f), 0.95f);

	if (UImage* ChancePanel = AddCaptureImage(TEXT("Image_ChancePanel"), LoadCaptureTexture(CaptureTexturePath(1)), FVector2D(330.f, -132.f), FVector2D(390.f, 113.f), 0.92f))
	{
		TargetOnlyWidgets.Add(ChancePanel);
	}

	Text_TargetName = AddCaptureText(TEXT("Text_TargetName"), FVector2D(224.f, -166.f), FVector2D(250.f, 28.f), 18);
	Text_Chance = AddCaptureText(TEXT("Text_Chance"), FVector2D(345.f, -118.f), FVector2D(120.f, 56.f), 42);
	Text_SphereCount = AddCaptureText(TEXT("Text_SphereCount"), FVector2D(270.f, -197.f), FVector2D(130.f, 30.f), 19);

	TargetOnlyWidgets.Add(Text_TargetName);
	TargetOnlyWidgets.Add(Text_Chance);
	TargetOnlyWidgets.Add(Text_SphereCount);

	bLayoutBuilt = true;
}

void UPWCaptureAimWidget::BindCaptureComponent(UPWPlayerCaptureComponent* InCaptureComponent)
{
	if (BoundCaptureComponent == InCaptureComponent)
	{
		return;
	}

	UnbindCaptureComponent();
	BoundCaptureComponent = InCaptureComponent;
	if (BoundCaptureComponent)
	{
		BoundCaptureComponent->OnCaptureAimInfoChanged.AddUniqueDynamic(this, &UPWCaptureAimWidget::HandleCaptureAimChanged);
	}
}

void UPWCaptureAimWidget::UnbindCaptureComponent()
{
	if (BoundCaptureComponent)
	{
		BoundCaptureComponent->OnCaptureAimInfoChanged.RemoveDynamic(this, &UPWCaptureAimWidget::HandleCaptureAimChanged);
		BoundCaptureComponent = nullptr;
	}
}

void UPWCaptureAimWidget::RefreshCaptureAim()
{
	const bool bVisible = BoundCaptureComponent && BoundCaptureComponent->IsCaptureAimVisible();
	SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	const bool bHasTarget = BoundCaptureComponent && BoundCaptureComponent->HasCaptureAimTarget();
	for (UWidget* TargetOnlyWidget : TargetOnlyWidgets)
	{
		if (TargetOnlyWidget)
		{
			TargetOnlyWidget->SetVisibility(bHasTarget ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		}
	}

	if (Text_TargetName)
	{
		Text_TargetName->SetText(BoundCaptureComponent ? BoundCaptureComponent->GetCaptureAimTargetNameText() : FText::GetEmpty());
	}

	if (Text_Chance)
	{
		Text_Chance->SetText(BoundCaptureComponent
			? FText::Format(NSLOCTEXT("PWCaptureAim", "ChancePercent", "{0}%"), FText::AsNumber(BoundCaptureComponent->GetCaptureAimChancePercent()))
			: FText::GetEmpty());
	}

	if (Text_SphereCount)
	{
		Text_SphereCount->SetText(BoundCaptureComponent
			? FText::Format(NSLOCTEXT("PWCaptureAim", "SphereCount", "CAPTURED  {0}"), FText::AsNumber(BoundCaptureComponent->GetThrowableCaptureSphereCount()))
			: FText::GetEmpty());
	}
}

UImage* UPWCaptureAimWidget::AddCaptureImage(const FName WidgetName, UTexture2D* Texture, const FVector2D Position, const FVector2D Size, float Opacity)
{
	if (!RootCanvas || !Texture)
	{
		return nullptr;
	}

	UImage* Image = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), WidgetName);
	Image->SetBrushFromTexture(Texture, true);
	Image->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity));

	UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(Image);
	CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CanvasSlot->SetPosition(Position);
	CanvasSlot->SetSize(Size);
	return Image;
}

UTextBlock* UPWCaptureAimWidget::AddCaptureText(const FName WidgetName, const FVector2D Position, const FVector2D Size, int32 FontSize)
{
	if (!RootCanvas)
	{
		return nullptr;
	}

	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName);
	TextBlock->SetColorAndOpacity(FSlateColor(CaptureTextColor()));
	TextBlock->SetJustification(ETextJustify::Center);
	TextBlock->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f));
	TextBlock->SetShadowOffset(FVector2D(2.f, 2.f));

	FSlateFontInfo FontInfo = TextBlock->GetFont();
	FontInfo.Size = FontSize;
	TextBlock->SetFont(FontInfo);

	UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(TextBlock);
	CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CanvasSlot->SetPosition(Position);
	CanvasSlot->SetSize(Size);
	return TextBlock;
}

UTexture2D* UPWCaptureAimWidget::LoadCaptureTexture(const TCHAR* TexturePath) const
{
	UObject* LoadedObject = FSoftObjectPath(TexturePath).TryLoad();
	UTexture2D* Texture = Cast<UTexture2D>(LoadedObject);
	if (!Texture)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWCaptureAim] Failed to load capture UI texture: %s"), TexturePath);
	}
	return Texture;
}
