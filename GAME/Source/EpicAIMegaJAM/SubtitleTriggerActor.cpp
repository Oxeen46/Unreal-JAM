#include "SubtitleTriggerActor.h"
#include "SubtitleSubsystem.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

ASubtitleTriggerActor::ASubtitleTriggerActor()
{
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);
	Trigger->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	Trigger->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
}

void ASubtitleTriggerActor::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ASubtitleTriggerActor::HandleBeginOverlap);
}

void ASubtitleTriggerActor::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled()) return;
	if (bOnlyOnce && bPlayed) return;

	USubtitleSubsystem* Subtitles = USubtitleSubsystem::Get(this);
	if (!Subtitles) return;

	bPlayed = true;
	if (Voice || !Speaker.IsEmpty())
	{
		Subtitles->Say(Speaker, Message, Voice, Duration);
	}
	else
	{
		Subtitles->ShowMessage(Message, Duration);
	}
}