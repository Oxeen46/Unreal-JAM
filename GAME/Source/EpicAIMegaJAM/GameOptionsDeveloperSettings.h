#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameOptionsDeveloperSettings.generated.h"

class USoundMix;
class USoundClass;
class USubtitleWidget;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Game Options"))
class EPICAIMEGAJAM_API UGameOptionsDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGameOptionsDeveloperSettings()
	{
		CategoryName = TEXT("Game");
	}

	// An empty Sound Mix asset is enough
	UPROPERTY(EditAnywhere, Config, Category="Audio")
	TSoftObjectPtr<USoundMix> VolumeMix;

	// Sounds assigned to this Sound Class follow the Music slider
	UPROPERTY(EditAnywhere, Config, Category="Audio")
	TSoftObjectPtr<USoundClass> MusicClass;

	// Sounds assigned to this Sound Class follow the SFX slider
	UPROPERTY(EditAnywhere, Config, Category="Audio")
	TSoftObjectPtr<USoundClass> SfxClass;

	// Widget Blueprint (child of USubtitleWidget) shown by the Subtitle Subsystem
	UPROPERTY(EditAnywhere, Config, Category="Subtitles")
	TSoftClassPtr<USubtitleWidget> SubtitleWidgetClass;
};