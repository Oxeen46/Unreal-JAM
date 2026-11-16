#include "SceneAmbientActor.h"
#include "GameAudio.h"
#include "StoryStateSubsystem.h"
#include "Sound/SoundClass.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

ASceneAmbientActor::ASceneAmbientActor()
{
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	Audio->SetupAttachment(Root);
	Audio->bAutoActivate = false;
	Audio->bIsUISound = false;
	PrimaryActorTick.bCanEverTick = false;
}

void ASceneAmbientActor::BeginPlay()
{
	Super::BeginPlay();

	Audio->bAllowSpatialization = bSpatial;
	if (bSpatial && Attenuation)
	{
		Audio->AttenuationSettings = Attenuation;
		Audio->bOverrideAttenuation = false;
	}
	if (USoundClass* Class = (Channel == ESceneAudioChannel::Music) ? GameAudio::GetMusicClass() : GameAudio::GetSfxClass())
	{
		Audio->SoundClassOverride = Class;
	}
	Audio->OnAudioFinished.AddDynamic(this, &ASceneAmbientActor::HandleFinished);

	if (UGameInstance* GI = GetGameInstance())
	{
		Story = GI->GetSubsystem<UStoryStateSubsystem>();
		if (Story.IsValid() && (StartWhenFact.IsValid() || StopWhenFact.IsValid()))
		{
			Story->OnFactChanged.AddDynamic(this, &ASceneAmbientActor::HandleFactChanged);
		}
	}
	Evaluate();
}

void ASceneAmbientActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(StartTimer);
	GetWorldTimerManager().ClearTimer(NextTimer);
	if (Story.IsValid())
	{
		Story->OnFactChanged.RemoveDynamic(this, &ASceneAmbientActor::HandleFactChanged);
	}
	if (Audio)
	{
		Audio->OnAudioFinished.RemoveDynamic(this, &ASceneAmbientActor::HandleFinished);
		Audio->Stop();
	}
	Super::EndPlay(EndPlayReason);
}

void ASceneAmbientActor::HandleFactChanged(FGameplayTag Fact)
{
	Evaluate();
}

void ASceneAmbientActor::Evaluate()
{
	bool bShouldRun = true;
	if (StartWhenFact.IsValid() && !(Story.IsValid() && Story->HasFact(StartWhenFact))) bShouldRun = false;
	if (StopWhenFact.IsValid() && Story.IsValid() && Story->HasFact(StopWhenFact)) bShouldRun = false;

	if (bShouldRun && !bRunning && !bStartPending)
	{
		bStartPending = true;
		GetWorldTimerManager().SetTimer(StartTimer, this, &ASceneAmbientActor::BeginRun, FMath::Max(StartDelay, 0.01f), false);
	}
	else if (!bShouldRun && (bRunning || bStartPending))
	{
		StopRun();
	}
}

void ASceneAmbientActor::BeginRun()
{
	bStartPending = false;
	bRunning = true;
	IntroIndex = 0;
	PlayNext();
}

void ASceneAmbientActor::StopRun()
{
	bRunning = false;
	bStartPending = false;
	GetWorldTimerManager().ClearTimer(StartTimer);
	GetWorldTimerManager().ClearTimer(NextTimer);
	if (Audio && Audio->IsPlaying())
	{
		Audio->FadeOut(FMath::Max(FadeOutTime, 0.01f), 0.f);
	}
}

void ASceneAmbientActor::PlayNext()
{
	if (!bRunning || !Audio) return;

	USoundBase* Sound = nullptr;
	bool bIntro = false;
	if (IntroSounds.IsValidIndex(IntroIndex))
	{
		Sound = IntroSounds[IntroIndex++];
		bIntro = true;
	}
	else
	{
		Sound = LoopSound;
	}
	if (!Sound)
	{
		if (bIntro) PlayNext(); // empty intro slot, skip it
		return;
	}

	Audio->SetSound(Sound);
	const bool bFirst = (IntroIndex <= 1);
	if (bFirst && FadeInTime > 0.f)
	{
		Audio->FadeIn(FadeInTime, Volume);
	}
	else
	{
		Audio->SetVolumeMultiplier(Volume);
		Audio->Play();
	}
}

void ASceneAmbientActor::HandleFinished()
{
	if (!bRunning) return;
	const bool bMoreIntro = IntroSounds.IsValidIndex(IntroIndex);
	const float Gap = bMoreIntro ? 0.f : FMath::Max(RepeatGap, 0.f);
	if (Gap > 0.f)
	{
		GetWorldTimerManager().SetTimer(NextTimer, this, &ASceneAmbientActor::PlayNext, Gap, false);
	}
	else
	{
		PlayNext();
	}
}
