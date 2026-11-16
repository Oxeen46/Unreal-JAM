#include "ClueActor.h"
#include "GameAudio.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "StoryStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "StoryStateSubsystem.h"

AClueActor::AClueActor()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}
void AClueActor::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UStoryStateSubsystem* Story = GI->GetSubsystem<UStoryStateSubsystem>())
		{
			if (Story->HasClue(ClueFact))
			{
				Destroy(); // already collected before the checkpoint
			}
		}
	}
}

void AClueActor::Interact_Implementation(AActor* Interactor)
{
	bool bFirstTime = true;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UStoryStateSubsystem* Story = GI->GetSubsystem<UStoryStateSubsystem>())
		{
			bFirstTime = !(ClueFact.IsValid() && Story->HasClue(ClueFact));
			Story->CollectClue(ClueFact, ClueTitle, ClueText, ClueIcon);
		}
	}

	if (bFirstTime)
	{
		GameAudio::ClueCollected(this);
		if (CollectSound)
		{
			GameAudio::PlaySfx2D(this, CollectSound);
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Yellow,
			FString::Printf(TEXT("Clue collected: %s"), *ClueTitle.ToString()));
	}

	if (bDestroyOnCollect)
	{
		Destroy();
	}
}
FText AClueActor::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return InteractionText;
}