#include "CrosshairWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Styling/SlateBrush.h"

bool UCrosshairWidget::Initialize()
{
	const bool bResult = Super::Initialize();
	if (!bResult || !WidgetTree || WidgetTree->RootWidget) return bResult;

	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	Dot = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Dot"));

	FSlateBrush Brush;
	Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
	Brush.TintColor = FSlateColor(FLinearColor::White);
	Brush.ImageSize = FVector2D(IdleSize, IdleSize);
	Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
	Brush.OutlineSettings.Color = FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.45f)); // thin dark rim so the dot stays readable on bright walls
	Brush.OutlineSettings.Width = 1.f;
	Dot->SetBrush(Brush);
	Dot->SetColorAndOpacity(IdleColor);
	Dot->SetDesiredSizeOverride(FVector2D(IdleSize, IdleSize));
	CurrentSize = IdleSize;

	if (UOverlaySlot* DotSlot = Cast<UOverlaySlot>(Root->AddChild(Dot)))
	{
		DotSlot->SetHorizontalAlignment(HAlign_Center);
		DotSlot->SetVerticalAlignment(VAlign_Center);
	}
	return bResult;
}

void UCrosshairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!Dot) return;

	const float TargetSize = bFocused ? FocusedSize : IdleSize;
	CurrentSize = FMath::FInterpTo(CurrentSize, TargetSize, InDeltaTime, 14.f);

	const float Range = FMath::Max(FocusedSize - IdleSize, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp((CurrentSize - IdleSize) / Range, 0.f, 1.f);

	Dot->SetDesiredSizeOverride(FVector2D(CurrentSize, CurrentSize));
	Dot->SetColorAndOpacity(FMath::Lerp(IdleColor, FocusedColor, Alpha));
}
