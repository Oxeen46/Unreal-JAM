#include "MainMenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AMainMenuPlayerController::AMainMenuPlayerController()
{
	// Optional defaults
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Only create the menu if we have a valid class assigned
	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);

		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport(0); // Z-order 0 is fine for main menu

			// Force mouse cursor on
			bShowMouseCursor = true;

			// Set input mode to UI only so the player can't move a character
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create MainMenuWidgetInstance"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuWidgetClass is not set on the Player Controller!"));
	}
}

void AMainMenuPlayerController::RemoveMainMenu()
{
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;

		// Switch back to game input mode if needed
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
}