#include "ClueActor.h"
#include "StoryStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "StoryStateSubsystem.h"

AClueActor::AClueActor()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

void AClueActor::Interact_Implementation(AActor* Interactor)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UStoryStateSubsystem* Story = GI->GetSubsystem<UStoryStateSubsystem>())
		{
			Story->CollectClue(ClueFact, ClueTitle);
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