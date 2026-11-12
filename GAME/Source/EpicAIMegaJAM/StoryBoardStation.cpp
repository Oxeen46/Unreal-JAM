#include "StoryBoardStation.h"
#include "StoryBoardData.h"
#include "StoryBoardWidget.h"
#include "StoryStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"

AStoryBoardStation::AStoryBoardStation()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

void AStoryBoardStation::Interact_Implementation(AActor* Interactor)
{
	if (ActiveWidget || !Board || !WidgetClass) return;

	if (bCompleted)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::White, TEXT("This story is already written."));
		}
		return;
	}

	APawn* Pawn = Cast<APawn>(Interactor);
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC) return;

	ActiveWidget = CreateWidget<UStoryBoardWidget>(PC, WidgetClass);
	if (!ActiveWidget) return;

	ActivePC = PC;
	ActiveWidget->InitBoard(Board, this);
	ActiveWidget->AddToViewport();

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(ActiveWidget->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(Mode);
	PC->SetShowMouseCursor(true);
}

bool AStoryBoardStation::TryCommit()
{
	UGameInstance* GI = GetGameInstance();
	UStoryStateSubsystem* Story = GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr;
	if (!Story || !Board) return false;

	if (Story->CommitBoard(Board))
	{
		bCompleted = true;
		return true;
	}
	return false;
}

void AStoryBoardStation::CloseBoard()
{
	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}
	if (ActivePC)
	{
		ActivePC->SetInputMode(FInputModeGameOnly());
		ActivePC->SetShowMouseCursor(false);
		ActivePC = nullptr;
	}
}