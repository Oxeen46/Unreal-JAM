#include "StoryDoor.h"
#include "StoryStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"

AStoryDoor::AStoryDoor()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

void AStoryDoor::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		Story = GI->GetSubsystem<UStoryStateSubsystem>();
		if (Story.IsValid())
		{
			Story->OnFactChanged.AddDynamic(this, &AStoryDoor::HandleFactChanged);
			Apply(Story->HasFact(RequiredFact));
		}
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
			FString::Printf(TEXT("Door BeginPlay: bound to story = %d"), Story.IsValid() ? 1 : 0));
	}
}

void AStoryDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Story.IsValid())
	{
		Story->OnFactChanged.RemoveDynamic(this, &AStoryDoor::HandleFactChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void AStoryDoor::HandleFactChanged(FGameplayTag Fact)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
			FString::Printf(TEXT("Door heard: %s"), *Fact.ToString()));
	}
	if (Fact == RequiredFact && Story.IsValid())
	{
		Apply(Story->HasFact(RequiredFact));
	}
}

void AStoryDoor::Apply(bool bUnlocked)
{
	SetActorHiddenInGame(bUnlocked);
	SetActorEnableCollision(!bUnlocked);
	OnStateChanged(bUnlocked);
}
void AStoryDoor::Interact_Implementation(AActor* Interactor)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
	FString::Printf(TEXT("It's locked. Needs: %s"), *RequiredFact.ToString()));
	}
}