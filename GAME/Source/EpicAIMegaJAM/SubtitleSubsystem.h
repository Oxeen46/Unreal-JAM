#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SubtitleSubsystem.generated.h"

class USubtitleWidget;
class USoundBase;

UCLASS()
class EPICAIMEGAJAM_API USubtitleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Easy access from C++
	static USubtitleSubsystem* Get(const UObject* WorldContext);

	// Gameplay text with no voice, e.g. "It won't open." Always shows, even with subtitles off.
	// Duration 0 = automatic.
	UFUNCTION(BlueprintCallable, Category="Subtitles")
	void ShowMessage(const FText& Text, float Duration = 0.f);

	// A spoken line. Plays the voice (if any) and shows the subtitle if subtitles are on.
	// Duration 0 = the length of the voice clip, or automatic from the text.
	UFUNCTION(BlueprintCallable, Category="Subtitles")
	void Say(const FText& Speaker, const FText& Text, USoundBase* Voice = nullptr, float Duration = 0.f);

	UFUNCTION(BlueprintCallable, Category="Subtitles")
	void ClearAll();

private:
	USubtitleWidget* GetWidget();
	void ApplyScale(USubtitleWidget* SubtitleWidget) const;

	TWeakObjectPtr<USubtitleWidget> Widget;
};