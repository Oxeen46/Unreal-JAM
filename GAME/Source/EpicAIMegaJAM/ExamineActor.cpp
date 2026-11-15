#include "ExamineActor.h"
#include "SubtitleSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AExamineActor::AExamineActor()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	if (CubeFinder.Succeeded()) Mesh->SetStaticMesh(CubeFinder.Object);
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

FText AExamineActor::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return InteractionText;
}

void AExamineActor::Interact_Implementation(AActor* Interactor)
{
	USubtitleSubsystem* Subtitles = USubtitleSubsystem::Get(this);
	if (!Subtitles) return;

	if (Voice || !Speaker.IsEmpty())
	{
		Subtitles->Say(Speaker, Message, Voice, Duration);
	}
	else
	{
		Subtitles->ShowMessage(Message, Duration);
	}
}