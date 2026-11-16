#include "KeyHintWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"

bool UKeyHintWidget::Initialize()
{
	const bool bResult = Super::Initialize();
	if (!bResult || !WidgetTree || WidgetTree->RootWidget) return bResult;

	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Row"));
	if (UOverlaySlot* RowSlot = Cast<UOverlaySlot>(Root->AddChild(Row)))
	{
		RowSlot->SetHorizontalAlignment(HAlign_Left);
		RowSlot->SetVerticalAlignment(VAlign_Bottom);
		RowSlot->SetPadding(FMargin(48.f, 0.f, 0.f, 48.f));
	}

	// the key cap
	UBorder* Cap = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Cap"));
	FSlateBrush CapBrush;
	CapBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
	CapBrush.TintColor = FSlateColor(FLinearColor(0.07f, 0.07f, 0.07f, 0.92f));
	CapBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
	CapBrush.OutlineSettings.CornerRadii = FVector4(8.f, 8.f, 8.f, 8.f);
	CapBrush.OutlineSettings.Color = FSlateColor(FLinearColor(0.92f, 0.92f, 0.92f, 1.f));
	CapBrush.OutlineSettings.Width = 2.f;
	Cap->SetBrush(CapBrush);
	Cap->SetBrushColor(FLinearColor::White);
	Cap->SetPadding(FMargin(18.f, 8.f));

	KeyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("KeyText"));
	KeyText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 22));
	KeyText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	KeyText->SetText(FText::FromString(TEXT("Backspace")));
	Cap->SetContent(KeyText);

	if (UHorizontalBoxSlot* CapSlot = Cast<UHorizontalBoxSlot>(Row->AddChild(Cap)))
	{
		CapSlot->SetVerticalAlignment(VAlign_Center);
	}

	ActionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ActionText"));
	ActionText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 20));
	ActionText->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.92f, 0.92f, 1.f)));
	ActionText->SetText(FText::FromString(TEXT("Leave")));
	if (UHorizontalBoxSlot* TextSlot = Cast<UHorizontalBoxSlot>(Row->AddChild(ActionText)))
	{
		TextSlot->SetVerticalAlignment(VAlign_Center);
		TextSlot->SetPadding(FMargin(16.f, 0.f, 0.f, 0.f));
	}

	return bResult;
}

void UKeyHintWidget::SetTexts(const FText& Key, const FText& Action)
{
	if (KeyText) KeyText->SetText(Key);
	if (ActionText) ActionText->SetText(Action);
}
