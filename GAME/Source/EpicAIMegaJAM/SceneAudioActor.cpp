#include "SceneAudioActor.h"
#include "GameAudio.h"
#include "StoryStateSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundClass.h"
#include "Components/SceneComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

ASceneAudioActor::ASceneAudioActor()
{
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	Audio->SetupAttachment(Root);
	Audio->bAutoActivate = false;
	PrimaryActorTick.bCanEverTick = false;
}

FString ASceneAudioActor::PlayedId() const
{
	return FString::Printf(TEXT("Audio/%s/%s"), *UGameplayStatics::GetCurrentLevelName(this, true), *GetName());
}

void ASceneAudioActor::BeginPlay()
{
	Super::BeginPlay();

	// Put the sound on the Music or SFX class so the options menu sliders control it
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

	// Already played before the checkpoint we loaded from
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UStoryStateSubsystem* Story = GI->GetSubsystem<UStoryStateSubsystem>())
		{
			if (Story->IsPickupCollected(PlayedId()))
			{
				bPlayed = true;
				return;
			}
		}
	}

	if (Trigger == ESceneAudioTrigger::AfterSeconds)
	{
		GetWorldTimerManager().SetTimer(StartTimer, this, &ASceneAudioActor::StartSequence, FMath::Max(StartDelay, 0.01f), false);
	}
	else
	{
		GetWorldTimerManager().SetTimer(CheckTimer, this, &ASceneAudioActor::CheckApproach, 0.1f, true);
	}
}

void ASceneAudioActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(CheckTimer);
	GetWorldTimerManager().ClearTimer(StartTimer);
	GetWorldTimerManager().ClearTimer(StepTimer);
	Super::EndPlay(EndPlayReason);
}

void ASceneAudioActor::CheckApproach()
{
	if (bPlayed) return;

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn) return;

	const AActor* Target = ProximityTarget ? ProximityTarget.Get() : this;
	const FBox Box = Target->GetComponentsBoundingBox(true);
	const FVector P = Pawn->GetActorLocation();
	const float Dist = Box.IsValid ? FMath::Sqrt(Box.ComputeSquaredDistanceToPoint(P)) : FVector::Dist(P, Target->GetActorLocation());
	if (Dist <= TriggerDistance)
	{
		StartSequence();
	}
}

void ASceneAudioActor::StartSequence()
{
	if (bPlayed) return;
	bPlayed = true;
	GetWorldTimerManager().ClearTimer(CheckTimer);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UStoryStateSubsystem* Story = GI->GetSubsystem<UStoryStateSubsystem>())
		{
			Story->MarkPickupCollected(PlayedId());
		}
	}

	StepIndex = 0;
	PlayStep();
}

void ASceneAudioActor::PlayStep()
{
	if (!Steps.IsValidIndex(StepIndex)) return;
	const FSceneAudioStep& Step = Steps[StepIndex];
	++StepIndex;

	float Wait = 0.f;
	UE_LOG(LogTemp, Log, TEXT("SceneAudio: %s plays step %d (%s)"), *GetName(), StepIndex, Step.Sound ? *Step.Sound->GetName() : TEXT("none"));
	if (Step.Sound)
	{
		if (Audio)
		{
			Audio->SetSound(Step.Sound);
			Audio->SetVolumeMultiplier(Step.Volume);
			Audio->Play();
		}
		Wait = FMath::Max(Step.Sound->GetDuration(), 0.f);
	}
	Wait += FMath::Max(Step.GapAfter, 0.f);

	if (Steps.IsValidIndex(StepIndex))
	{
		GetWorldTimerManager().SetTimer(StepTimer, this, &ASceneAudioActor::PlayStep, FMath::Max(Wait, 0.05f), false);
	}
}
