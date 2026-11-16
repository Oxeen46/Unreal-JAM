#include "ChapterTitleWidget.h"
#include "Components/TextBlock.h"

void UChapterTitleWidget::SetChapterTitle(const FText& Title)
{
	if (ChapterText)
	{
		ChapterText->SetText(Title);
	}
}

void UChapterTitleWidget::PlayChapterAnimation()
{
	if (ChapterAnimation)
	{
		PlayAnimationForward(ChapterAnimation);
	}
}