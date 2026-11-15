#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChapterTitleWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class EPICAIMEGAJAM_API UChapterTitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetChapterTitle(const FText& Title);

	UFUNCTION(BlueprintCallable)
	void PlayChapterAnimation();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ChapterText;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> ChapterAnimation;
};