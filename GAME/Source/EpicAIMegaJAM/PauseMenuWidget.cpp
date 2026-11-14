#include "PauseMenuWidget.h"
#include "SaveSlotMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "InputCoreTypes.h"
#include "Engine/Engine.h"

void UPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);

	if (ResumeButton) ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleResume);
	if (LoadButton) LoadButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleLoad);
	if (QuitButton) QuitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleQuit);

	if ((!ResumeButton || !LoadButton || !QuitButton) && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
			TEXT("WBP_PauseMenu needs buttons named ResumeButton, LoadButton and QuitButton (Is Variable ticked)."));
	}
}

FReply UPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::P || Key == EKeys::Gamepad_Special_Right)
	{
		// Escape / P closes the load menu first if it's open, otherwise resumes
		if (LoadMenu) HandleLoadMenuClosed();
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
	LoadMenu->AddToViewport(10);
}

void UPauseMenuWidget::HandleLoadMenuClosed()
{
	if (LoadMenu)
	{
		LoadMenu->RemoveFromParent();
		LoadMenu = nullptr;
	}
}

void UPauseMenuWidget::HandleQuit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}