#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class USaveSlotMenuWidget;
class UOptionsMenuWidget;

UCLASS()
class EPICAIMEGAJAM_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Fired when the player wants to resume. The owner closes the menu.
	FSimpleDelegate OnResumeRequested;

	// Set these in WBP_PauseMenu's Class Defaults
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Menu")
	TSubclassOf<USaveSlotMenuWidget> LoadMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Menu")
	TSubclassOf<UOptionsMenuWidget> OptionsMenuClass;

	// The name of your main menu level (just the name, no path)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Menu")
	FName MainMenuLevelName;

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> ResumeButton;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> LoadButton;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> QuitButton; // goes back to the main menu

	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> OptionsButton;

private:
	UFUNCTION() void HandleResume();
	UFUNCTION() void HandleLoad();
	UFUNCTION() void HandleOptions();
	UFUNCTION() void HandleQuit();

	UFUNCTION() void HandleOptionsBack();
	void HandleLoadMenuClosed();

	UPROPERTY() TObjectPtr<USaveSlotMenuWidget> LoadMenu;
	UPROPERTY() TObjectPtr<UOptionsMenuWidget> OptionsMenu;
};