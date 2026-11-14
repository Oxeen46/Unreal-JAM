#include "PauseMenuWidget.h"
#include "SaveSlotMenuWidget.h"
#include "OptionsMenuWidget.h"
#include "StoryStateSubsystem.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"

void UPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);

	if (ResumeButton) ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleResume);
	if (LoadButton) LoadButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleLoad);
	if (OptionsButton) OptionsButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleOptions);
	if (QuitButton) QuitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleQuit);
}

FReply UPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::P || Key == EKeys::Gamepad_Special_Right)
	{
		// Close whatever is open on top first, otherwise resume
		if (OptionsMenu) HandleOptionsBack();
		else if (LoadMenu) HandleLoadMenuClosed();
		else HandleResume();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPauseMenuWidget::HandleResume()
{
	OnResumeRequested.ExecuteIfBound();
}

void UPauseMenuWidget::HandleLoad()
{
	if (!LoadMenuClass || LoadMenu) return;

	LoadMenu = CreateWidget<USaveSlotMenuWidget>(GetOwningPlayer(), LoadMenuClass);
	if (!LoadMenu) return;

	LoadMenu->InitMenu(ESlotMenuMode::Load);
	LoadMenu->OnClosed.BindUObject(this, &UPauseMenuWidget::HandleLoadMenuClosed);
	LoadMenu->AddToViewport(60);
}

void UPauseMenuWidget::HandleLoadMenuClosed()
{
	if (LoadMenu)
	{
		LoadMenu->RemoveFromParent();
		LoadMenu = nullptr;
	}
}

void UPauseMenuWidget::HandleOptions()
{
	if (OptionsMenu) return;

	if (!OptionsMenuClass)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT("Pause menu needs an Options Menu Class (set it in WBP_PauseMenu's Class Defaults)."));
		}
		return;
	}

	OptionsMenu = CreateWidget<UOptionsMenuWidget>(GetOwningPlayer(), OptionsMenuClass);
	if (!OptionsMenu) return;

	OptionsMenu->OnBack.AddDynamic(this, &UPauseMenuWidget::HandleOptionsBack);
	OptionsMenu->AddToViewport(60);
	OptionsMenu->SetUserFocus(GetOwningPlayer());

	SetVisibility(ESlateVisibility::Collapsed); // hide the pause menu while options are open
}

void UPauseMenuWidget::HandleOptionsBack()
{
	if (OptionsMenu)
	{
		OptionsMenu->RemoveFromParent();
		OptionsMenu = nullptr;
	}

	SetVisibility(ESlateVisibility::Visible);
	SetUserFocus(GetOwningPlayer());
}

// "Quit" now goes back to the main menu
void UPauseMenuWidget::HandleQuit()
{
	if (MainMenuLevelName.IsNone())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT("Pause menu needs a Main Menu Level Name (set it in WBP_PauseMenu's Class Defaults)."));
		}
		return;
	}

	// Unsaved progress is dropped. Saves on disk are untouched.
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (UStoryStateSubsystem* Story = GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr)
	{
		Story->ResetAllState();
	}

	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}