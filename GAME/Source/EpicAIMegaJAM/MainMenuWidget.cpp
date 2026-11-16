#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "OptionsMenuWidget.h"
#include "Engine/Engine.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartClicked);
	}

	if (OptionsButton)
	{
		OptionsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}
}

void UMainMenuWidget::OnStartClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}

	RemoveFromParent();
	UGameplayStatics::OpenLevel(this, GameLevelName);
}

void UMainMenuWidget::OnOptionsClicked()
{
	if (OptionsMenu) return; // already open

	if (!OptionsMenuClass)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT("Main menu needs an Options Menu Class (set it in WBP_MainMenu's Class Defaults)."));
		}
		return;
	}

	OptionsMenu = CreateWidget<UOptionsMenuWidget>(GetOwningPlayer(), OptionsMenuClass);
	if (!OptionsMenu) return;

	OptionsMenu->OnBack.AddDynamic(this, &UMainMenuWidget::HandleOptionsBack);
	OptionsMenu->AddToViewport(10);

	SetVisibility(ESlateVisibility::Collapsed); // hide the main menu while options are open
}

void UMainMenuWidget::HandleOptionsBack()
{
	if (OptionsMenu)
	{
		OptionsMenu->RemoveFromParent();
		OptionsMenu = nullptr;
	}

	SetVisibility(ESlateVisibility::Visible); // bring the main menu back
}

void UMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}