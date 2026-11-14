#include "CheckpointActor.h"
#include "StoryStateSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"

ACheckpointActor::ACheckpointActor()
{
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);
	Trigger->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	Trigger->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

	RespawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("RespawnPoint"));
	RespawnPoint->SetupAttachment(Trigger);
}

void ACheckpointActor::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ACheckpointActor::HandleBeginOverlap);
}

void ACheckpointActor::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (bUsed || !Pawn || !Pawn->IsPlayerControlled()) return;

	UGameInstance* GI = GetGameInstance();
	UStoryStateSubsystem* Story = GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr;
	if (!Story) return;

	bUsed = true;

	FTransform Spawn = RespawnPoint->GetComponentTransform();
	Spawn.SetScale3D(FVector::OneVector);
	Spawn.AddToTranslation(FVector(0.f, 0.f, 100.f)); // stand above the floor, not in it
	Story->SetCheckpoint(Spawn, PlaceName);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Checkpoint saved."));
	}
	OnCheckpointReached();
}