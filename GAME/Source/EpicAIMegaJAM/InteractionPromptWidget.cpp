#include "InteractionPromptWidget.h"
#include "Components/TextBlock.h"

void UInteractionPromptWidget::SetPrompt(const FText& Key, const FText& Action)
{
	if (!KeyText->GetText().EqualTo(Key)) KeyText->SetText(Key);
	if (!ActionText->GetText().EqualTo(Action)) ActionText->SetText(Action);
}