#include "WakeUpFadeActor.h"
#include "StoryStateSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AWakeUpFadeActor::AWakeUpFadeActor()
{
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

FString AWakeUpFadeActor::PlayedId() const
{
	return FString::Printf(TEXT("WakeUp/%s/%s"), *UGameplayStatics::GetCurrentLevelName(this, true), *GetName());
}

void AWakeUpFadeActor::BeginPlay()
{
	Super::BeginPlay();

	UStoryStateSubsystem* Story = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		Story = GI->GetSubsystem<UStoryStateSubsystem>();
	}
	if (bOnlyOncePerRun && Story)
	{
		if (Story->IsPickupCollected(PlayedId())) return; // already woke up before the checkpoint
		Story->MarkPickupCollected(PlayedId());
	}

	bActive = true;
	Elapsed = 0.f;
	Apply(1.f); // black from the very first frame
	SetActorTickEnabled(true);
}

void AWakeUpFadeActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bActive) Apply(0.f);
	Super::EndPlay(EndPlayReason);
}

float AWakeUpFadeActor::AlphaAt(float Time) const
{
	if (Time <= HoldBlack) return 1.f;

	const float U = FMath::Clamp((Time - HoldBlack) / FMath::Max(OpenDuration, 0.1f), 0.f, 1.f);
	const float Base = 1.f - FMath::SmoothStep(0.f, 1.f, U);

	// heavy eyelids: the screen darkens again around a few moments while opening
	float Dip = 0.f;
	const int32 N = FMath::Max(Blinks, 0);
	for (int32 i = 1; i <= N; ++i)
	{
		const float Center = 0.7f * float(i) / float(N + 1) + 0.1f;
		const float D = (U - Center) / 0.05f;
		Dip += FMath::Exp(-D * D);
	}
	return FMath::Clamp(Base + BlinkDepth * Dip * (1.f - U), 0.f, 1.f);
}

void AWakeUpFadeActor::Apply(float Alpha) const
{
	if (APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		Cam->SetManualCameraFade(Alpha, FLinearColor::Black, false);
	}
}

void AWakeUpFadeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bActive) return;

	Elapsed += DeltaTime;
	const float Alpha = AlphaAt(Elapsed);
	Apply(Alpha);

	if (Elapsed >= HoldBlack + OpenDuration)
	{
		Apply(0.f);
		bActive = false;
		SetActorTickEnabled(false);
	}
}
