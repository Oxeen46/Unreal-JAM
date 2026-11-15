#include "NoteCardWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UNoteCardWidget::SetNote(UTexture2D* Icon, const FText& Name, int32 Order)
{
	if (IconImage)
	{
		if (Icon)
		{
			IconImage->SetBrushFromTexture(Icon);
			IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			IconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (NameText) NameText->SetText(Name);
	if (OrderText) OrderText->SetText(FText::FromString(FString::Printf(TEXT("#%d"), Order)));
}