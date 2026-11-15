#include "SubtitleSubsystem.h"
#include "SubtitleWidget.h"
#include "GameSettingsSubsystem.h"
#include "GameOptionsDeveloperSettings.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

namespace
{
	float AutoDuration(const FText& Text, USoundBase* Voice, float Requested)
	{
		if (Requested > 0.f) return Requested;

		if (Voice)
		{
			const float VoiceLength = Voice->GetDuration();
			if (VoiceLength > 0.f && VoiceLength < 120.f) return VoiceLength + 0.4f;
		}
		return FMath::Clamp(Text.ToString().Len() * 0.055f + 1.4f, 2.f, 9.f);
	}
}

USubtitleSubsystem* USubtitleSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<USubtitleSubsystem>() : nullptr;
}

USubtitleWidget* USubtitleSubsystem::GetWidget()
{
	if (Widget.IsValid()) return Widget.Get();

	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	const UGameOptionsDeveloperSettings* Config = GetDefault<UGameOptionsDeveloperSettings>();
	UClass* WidgetClass = Config ? Config->SubtitleWidgetClass.LoadSynchronous() : nullptr;
	if (!PC || !WidgetClass) return nullptr;

	USubtitleWidget* NewWidget = CreateWidget<USubtitleWidget>(PC, WidgetClass);
	if (!NewWidget) return nullptr;

	NewWidget->AddToViewport(20);
	NewWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	Widget = NewWidget;
	return NewWidget;
}

void USubtitleSubsystem::ApplyScale(USubtitleWidget* SubtitleWidget) const
{
	UGameInstance* GI = GetGameInstance();
	if (const UGameSettingsSubsystem* Settings = GI ? GI->GetSubsystem<UGameSettingsSubsystem>() : nullptr)
	{
		SubtitleWidget->SetTextScale(Settings->GetOptions().SubtitleScale);
	}
}

void USubtitleSubsystem::ShowMessage(const FText& Text, float Duration)
{
	if (Text.IsEmpty()) return;

	const float Length = AutoDuration(Text, nullptr, Duration);
	if (USubtitleWidget* SubtitleWidget = GetWidget())
	{
		ApplyScale(SubtitleWidget);
		SubtitleWidget->Enqueue(FText::GetEmpty(), Text, Length, nullptr, true);
	}
	else if (GEngine)
	{
		// No subtitle widget set up yet (Project Settings > Game > Game Options)
		GEngine->AddOnScreenDebugMessage(-1, Length, FColor::Yellow, Text.ToString());
	}
}

void USubtitleSubsystem::Say(const FText& Speaker, const FText& Text, USoundBase* Voice, float Duration)
{
	UGameInstance* GI = GetGameInstance();
	const UGameSettingsSubsystem* Settings = GI ? GI->GetSubsystem<UGameSettingsSubsystem>() : nullptr;
	const bool bSubtitlesOn = Settings ? Settings->GetOptions().bSubtitles : true;

	// Subtitles off: just play the voice
	if (!bSubtitlesOn)
	{
		if (Voice) UGameplayStatics::PlaySound2D(GetWorld(), Voice);
		return;
	}

	const float Length = AutoDuration(Text, Voice, Duration);
	if (USubtitleWidget* SubtitleWidget = GetWidget())
	{
		ApplyScale(SubtitleWidget);
		SubtitleWidget->Enqueue(Speaker, Text, Length, Voice, false);
	}
	else
	{
		if (Voice) UGameplayStatics::PlaySound2D(GetWorld(), Voice);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, Length, FColor::Yellow,
				FString::Printf(TEXT("%s: %s"), *Speaker.ToString(), *Text.ToString()));
		}
	}
}

void USubtitleSubsystem::ClearAll()
{
	if (Widget.IsValid())
	{
		Widget->ClearAll();
	}
}