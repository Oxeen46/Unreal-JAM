#include "SavePointActor.h"
#include "SaveSlotMenuWidget.h"
#include "StoryStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"

ASavePointActor::ASavePointActor()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	if (CubeFinder.Succeeded()) Mesh->SetStaticMesh(CubeFinder.Object);
	Mesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.f));
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	RespawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("RespawnPoint"));
	RespawnPoint->SetupAttachment(Mesh);
	RespawnPoint->SetRelativeLocation(FVector(300.f, 0.f, 0.f));
}

FText ASavePointActor::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return Menu ? FText::GetEmpty() : InteractionText;
}

void ASavePointActor::Interact_Implementation(AActor* Interactor)
{
	if (Menu) return;

	if (!SaveMenuClass)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT("Save point needs a Save Menu Class."));
		}
		return;
	}

	const APawn* Pawn = Cast<APawn>(Interactor);
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	UGameInstance* GI = GetGameInstance();
	UStoryStateSubsystem* Story = GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr;
	if (!PC || !Story) return;

	// A save point is also a checkpoint
	FTransform Spawn = RespawnPoint->GetComponentTransform();
	Spawn.SetScale3D(FVector::OneVector);
	Spawn.AddToTranslation(FVector(0.f, 0.f, 100.f));
	Story->SetCheckpoint(Spawn, PlaceName);

	Menu = CreateWidget<USaveSlotMenuWidget>(PC, SaveMenuClass);
	if (!Menu) return;

	MenuPC = PC;
	Menu->InitMenu(ESlotMenuMode::Save);
	Menu->OnClosed.BindUObject(this, &ASavePointActor::CloseMenu);
	Menu->AddToViewport(50);

	UGameplayStatics::SetGamePaused(GetWorld(), true);

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(Menu->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(Mode);
	PC->SetShowMouseCursor(true);
}

void ASavePointActor::CloseMenu()
{
	if (Menu)
	{
		Menu->RemoveFromParent();
		Menu = nullptr;
	}

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (MenuPC)
	{
		MenuPC->SetInputMode(FInputModeGameOnly());
		MenuPC->SetShowMouseCursor(false);
		MenuPC = nullptr;
	}
}