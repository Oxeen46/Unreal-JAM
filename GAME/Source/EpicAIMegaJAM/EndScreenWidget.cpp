#include "EndScreenWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"

bool UEndScreenWidget::Initialize()
{
	const bool bResult = Super::Initialize();
	if (!bResult || !WidgetTree || WidgetTree->RootWidget) return bResult;

	UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;
	Root->SetBrush(FSlateColorBrush(FLinearColor::Black));
	Root->SetBrushColor(FLinearColor::Black);
	Root->SetHorizontalAlignment(HAlign_Center);
	Root->SetVerticalAlignment(VAlign_Center);

	MessageText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Message"));
	MessageText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 48));
	MessageText->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.9f, 0.84f, 1.f)));
	MessageText->SetJustification(ETextJustify::Center);
	Root->SetContent(MessageText);

	SetRenderOpacity(0.f);
	return bResult;
}

void UEndScreenWidget::Show(const FText& Message, float FadeTime)
{
	if (MessageText) MessageText->SetText(Message);
	Fade = FMath::Max(FadeTime, 0.01f);
	Elapsed = 0.f;
	SetRenderOpacity(0.f);
}

void UEndScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (Elapsed < Fade)
	{
		Elapsed += InDeltaTime;
		SetRenderOpacity(FMath::SmoothStep(0.f, 1.f, FMath::Clamp(Elapsed / Fade, 0.f, 1.f)));
	}
}
