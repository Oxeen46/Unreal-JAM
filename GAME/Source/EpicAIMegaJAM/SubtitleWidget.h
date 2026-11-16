#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SubtitleWidget.generated.h"

class UWidget;
class UTextBlock;
class USoundBase;

UCLASS()
class EPICAIMEGAJAM_API USubtitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Plain messages replace other plain messages right away. Spoken lines queue up.
	void Enqueue(const FText& Speaker, const FText& Line, float Duration, USoundBase* Voice, bool bIsMessage);
	void ClearAll();
	void SetTextScale(float Scale);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// The panel holding the text. It is scaled by the Subtitle Size option.
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UWidget> LineBox;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> SpeakerText;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> LineText;

private:
	struct FEntry
	{
		FText Speaker;
		FText Line;
		float Duration = 2.f;
		TWeakObjectPtr<USoundBase> Voice;
	};

	void StartNext();

	TArray<FEntry> Queue;
	FEntry Current;
	bool bHasCurrent = false;
	float TimeLeft = 0.f;
	float Alpha = 0.f;
};