#include "UI/PW_BuildingRadialMenuWidget.h"

#include "Base/PW_PlayerBuildingPlacementComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"
#include "Widgets/SWidget.h"

TSharedRef<SWidget> UPW_BuildingRadialMenuWidget::RebuildWidget()
{
	bool bShouldBuildDefaultLayout = WidgetTree && WidgetTree->RootWidget == nullptr;
	if (!bShouldBuildDefaultLayout && WidgetTree)
	{
		const UPanelWidget* RootPanel = Cast<UPanelWidget>(WidgetTree->RootWidget);
		bShouldBuildDefaultLayout = RootPanel != nullptr && RootPanel->GetChildrenCount() == 0;
	}

	if (bShouldBuildDefaultLayout)
	{
		BuildDefaultLayout();
	}

	return Super::RebuildWidget();
}

void UPW_BuildingRadialMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateDefaultTexts();
}

void UPW_BuildingRadialMenuWidget::InitializeWithBuildingPlacementComponent(UPW_PlayerBuildingPlacementComponent* InPlacementComponent)
{
	PlacementComponent = InPlacementComponent;
	UpdateDefaultTexts();
	BP_OnMaterialPageChanged(CurrentMaterialType);
}

void UPW_BuildingRadialMenuWidget::CycleMaterialPage()
{
	switch (CurrentMaterialType)
	{
	case EPW_BuildingMaterialType::Wood:
		SetCurrentMaterialType(EPW_BuildingMaterialType::Stone);
		break;
	case EPW_BuildingMaterialType::Stone:
		SetCurrentMaterialType(EPW_BuildingMaterialType::Iron);
		break;
	case EPW_BuildingMaterialType::Iron:
	default:
		SetCurrentMaterialType(EPW_BuildingMaterialType::Wood);
		break;
	}
}

void UPW_BuildingRadialMenuWidget::SetCurrentMaterialType(EPW_BuildingMaterialType InMaterialType)
{
	if (CurrentMaterialType == InMaterialType)
	{
		return;
	}

	CurrentMaterialType = InMaterialType;
	UpdateDefaultTexts();
	BP_OnMaterialPageChanged(CurrentMaterialType);
}

void UPW_BuildingRadialMenuWidget::SelectFoundation()
{
	SelectPiece(EPW_BuildingPieceType::Foundation);
}

void UPW_BuildingRadialMenuWidget::SelectWall()
{
	SelectPiece(EPW_BuildingPieceType::Wall);
}

void UPW_BuildingRadialMenuWidget::SelectRoof()
{
	SelectPiece(EPW_BuildingPieceType::Roof);
}

void UPW_BuildingRadialMenuWidget::CancelBuilding()
{
	if (PlacementComponent)
	{
		PlacementComponent->CancelPlacement();
	}
}

void UPW_BuildingRadialMenuWidget::SelectPiece(EPW_BuildingPieceType PieceType)
{
	if (PlacementComponent)
	{
		PlacementComponent->EnterPlacementMode(PieceType, CurrentMaterialType);
	}

	BP_OnBuildingPieceSelected(PieceType, CurrentMaterialType);
	UpdateDefaultTexts();
	SetVisibility(ESlateVisibility::Collapsed);

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}

void UPW_BuildingRadialMenuWidget::BuildDefaultLayout()
{
	if (!WidgetTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("PW_BuildingRadialMenuWidget cannot build default layout because WidgetTree is null."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("PW_BuildingRadialMenuWidget building default layout."));

	DefaultRootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("BuildingRadialRoot"));
	WidgetTree->RootWidget = DefaultRootCanvas;

	DefaultRadialImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("BuildingRadialImage"));
	if (UTexture2D* RadialTexture = LoadObject<UTexture2D>(
		nullptr,
		TEXT("/Game/_Private/LMK/UI/Building/T_BuildingRadialMenuBase.T_BuildingRadialMenuBase")))
	{
		FSlateBrush RadialBrush;
		RadialBrush.SetResourceObject(RadialTexture);
		RadialBrush.ImageSize = FVector2D(720.0f, 720.0f);
		DefaultRadialImage->SetBrush(RadialBrush);
	}
	DefaultRootCanvas->AddChild(DefaultRadialImage);
	if (UCanvasPanelSlot* RadialSlot = Cast<UCanvasPanelSlot>(DefaultRadialImage->Slot))
	{
		RadialSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		RadialSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		RadialSlot->SetPosition(FVector2D(0.0f, 0.0f));
		RadialSlot->SetSize(FVector2D(720.0f, 720.0f));
	}

	DefaultTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BuildingTitleText"));
	DefaultTitleText->SetJustification(ETextJustify::Center);
	DefaultTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.78f, 0.36f, 1.0f)));
	DefaultRootCanvas->AddChild(DefaultTitleText);
	if (UCanvasPanelSlot* TitleSlot = Cast<UCanvasPanelSlot>(DefaultTitleText->Slot))
	{
		TitleSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		TitleSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		TitleSlot->SetPosition(FVector2D(0.0f, -350.0f));
		TitleSlot->SetSize(FVector2D(420.0f, 48.0f));
	}

	DefaultMaterialText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BuildingMaterialText"));
	DefaultMaterialText->SetJustification(ETextJustify::Center);
	DefaultMaterialText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.88f, 0.72f, 1.0f)));
	DefaultRootCanvas->AddChild(DefaultMaterialText);
	if (UCanvasPanelSlot* MaterialSlot = Cast<UCanvasPanelSlot>(DefaultMaterialText->Slot))
	{
		MaterialSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		MaterialSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		MaterialSlot->SetPosition(FVector2D(0.0f, -300.0f));
		MaterialSlot->SetSize(FVector2D(420.0f, 36.0f));
	}

	DefaultSelectedText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BuildingSelectedText"));
	DefaultSelectedText->SetJustification(ETextJustify::Center);
	DefaultSelectedText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.78f, 0.36f, 1.0f)));
	DefaultRootCanvas->AddChild(DefaultSelectedText);
	if (UCanvasPanelSlot* SelectedSlot = Cast<UCanvasPanelSlot>(DefaultSelectedText->Slot))
	{
		SelectedSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		SelectedSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		SelectedSlot->SetPosition(FVector2D(0.0f, 190.0f));
		SelectedSlot->SetSize(FVector2D(420.0f, 48.0f));
	}

	UButton* FoundationButton = CreateDefaultButton(NSLOCTEXT("PWBuilding", "FoundationButton", "Foundation"), FVector2D(0.0f, -210.0f), FVector2D(140.0f, 78.0f));
	UButton* WallButton = CreateDefaultButton(NSLOCTEXT("PWBuilding", "WallButton", "Wall"), FVector2D(190.0f, -55.0f), FVector2D(120.0f, 72.0f));
	UButton* RoofButton = CreateDefaultButton(NSLOCTEXT("PWBuilding", "RoofButton", "Roof"), FVector2D(-190.0f, -55.0f), FVector2D(120.0f, 72.0f));
	UButton* MaterialButton = CreateDefaultButton(NSLOCTEXT("PWBuilding", "MaterialNextButton", ">"), FVector2D(310.0f, 0.0f), FVector2D(82.0f, 110.0f));

	if (FoundationButton)
	{
		FoundationButton->OnClicked.AddDynamic(this, &UPW_BuildingRadialMenuWidget::HandleFoundationClicked);
	}
	if (WallButton)
	{
		WallButton->OnClicked.AddDynamic(this, &UPW_BuildingRadialMenuWidget::HandleWallClicked);
	}
	if (RoofButton)
	{
		RoofButton->OnClicked.AddDynamic(this, &UPW_BuildingRadialMenuWidget::HandleRoofClicked);
	}
	if (MaterialButton)
	{
		MaterialButton->OnClicked.AddDynamic(this, &UPW_BuildingRadialMenuWidget::HandleCycleMaterialClicked);
	}
}

void UPW_BuildingRadialMenuWidget::UpdateDefaultTexts()
{
	if (DefaultTitleText)
	{
		DefaultTitleText->SetText(FText::Format(NSLOCTEXT("PWBuilding", "DefaultBuildingTitle", "Building - {0}"), GetMaterialStructureText()));
	}

	if (DefaultMaterialText)
	{
		DefaultMaterialText->SetText(FText::Format(NSLOCTEXT("PWBuilding", "DefaultMaterialText", "MATERIAL: < {0} >"), GetMaterialDisplayText()));
	}

	if (DefaultSelectedText)
	{
		DefaultSelectedText->SetText(FText::Format(NSLOCTEXT("PWBuilding", "DefaultSelectedText", "SELECT STRUCTURE TYPE: {0}"), GetMaterialDisplayText()));
	}
}

FText UPW_BuildingRadialMenuWidget::GetMaterialDisplayText() const
{
	switch (CurrentMaterialType)
	{
	case EPW_BuildingMaterialType::Stone:
		return NSLOCTEXT("PWBuilding", "MaterialStone", "STONE");
	case EPW_BuildingMaterialType::Iron:
		return NSLOCTEXT("PWBuilding", "MaterialIron", "IRON");
	case EPW_BuildingMaterialType::Wood:
	default:
		return NSLOCTEXT("PWBuilding", "MaterialWood", "WOOD");
	}
}

FText UPW_BuildingRadialMenuWidget::GetMaterialStructureText() const
{
	switch (CurrentMaterialType)
	{
	case EPW_BuildingMaterialType::Stone:
		return NSLOCTEXT("PWBuilding", "StructureStone", "Stone Structures");
	case EPW_BuildingMaterialType::Iron:
		return NSLOCTEXT("PWBuilding", "StructureIron", "Iron Structures");
	case EPW_BuildingMaterialType::Wood:
	default:
		return NSLOCTEXT("PWBuilding", "StructureWood", "Wood Structures");
	}
}

UButton* UPW_BuildingRadialMenuWidget::CreateDefaultButton(const FText& Label, const FVector2D& Position, const FVector2D& Size)
{
	if (!WidgetTree || !DefaultRootCanvas)
	{
		return nullptr;
	}

	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	ButtonText->SetText(Label);
	ButtonText->SetJustification(ETextJustify::Center);
	ButtonText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.86f, 0.58f, 1.0f)));
	Button->SetContent(ButtonText);
	DefaultRootCanvas->AddChild(Button);

	if (UCanvasPanelSlot* ButtonSlot = Cast<UCanvasPanelSlot>(Button->Slot))
	{
		ButtonSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		ButtonSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		ButtonSlot->SetPosition(Position);
		ButtonSlot->SetSize(Size);
	}

	return Button;
}

void UPW_BuildingRadialMenuWidget::HandleFoundationClicked()
{
	SelectFoundation();
}

void UPW_BuildingRadialMenuWidget::HandleWallClicked()
{
	SelectWall();
}

void UPW_BuildingRadialMenuWidget::HandleRoofClicked()
{
	SelectRoof();
}

void UPW_BuildingRadialMenuWidget::HandleCycleMaterialClicked()
{
	CycleMaterialPage();
}
