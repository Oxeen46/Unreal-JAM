#include "SubtitleWidget.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void USubtitleWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetRenderOpacity(0.f);
	if (LineBox) LineBox->SetRenderTransformPivot(FVector2D(0.5f, 1.f));
}

void USubtitleWidget::SetTextScale(float Scale)
{
	if (LineBox) LineBox->SetRenderScale(FVector2D(Scale, Scale));
}

void USubtitleWidget::ClearAll()
{
	Queue.Reset();
	bHasCurrent = false;
	TimeLeft = 0.f;
}

void USubtitleWidget::Enqueue(const FText& Speaker, const FText& Line, float Duration, USoundBase* Voice, bool bIsMessage)
{
	// The same line again: just keep it up a little longer
	if (bHasCurrent && Current.Line.EqualTo(Line))
	{
		TimeLeft = FMath::Max(TimeLeft, Duration);
		return;
	}
	for (const FEntry& Queued : Queue)
	{
		if (Queued.Line.EqualTo(Line)) return;
	}

	FEntry Entry;
	Entry.Speaker = Speaker;
	Entry.Line = Line;
	Entry.Duration = Duration;
	Entry.Voice = Voice;

	// A plain message cuts in over another plain message, but never over spoken dialogue
	const bool bCurrentIsDialogue = bHasCurrent && (!Current.Speaker.IsEmpty() || Current.Voice.IsValid());
	if (bIsMessage && bHasCurrent && !bCurrentIsDialogue)
	{
		Queue.Insert(Entry, 0);
		TimeLeft = 0.f; // the current one ends on the next tick
		return;
	}

	Queue.Add(Entry);
	while (Queue.Num() > 6)
	{
		Queue.RemoveAt(0);
	}
}

void USubtitleWidget::StartNext()
{
	if (Queue.Num() == 0) return;

	Current = Queue[0];
	Queue.RemoveAt(0);
	bHasCurrent = true;
	TimeLeft = Current.Duration;

	if (SpeakerText)
	{
		SpeakerText->SetText(Current.Speaker);
		SpeakerText->SetVisibility(Current.Speaker.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (LineText) LineText->SetText(Current.Line);

	if (USoundBase* Sound = Current.Voice.Get())
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}

void USubtitleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bHasCurrent)
	{
		TimeLeft -= InDeltaTime;
		if (TimeLeft <= 0.f) bHasCurrent = false;
	}
	if (!bHasCurrent && Queue.Num() > 0)
	{
		StartNext();
	}

	Alpha = FMath::FInterpConstantTo(Alpha, bHasCurrent ? 1.f : 0.f, InDeltaTime, 8.f);
	SetRenderOpacity(Alpha);
}