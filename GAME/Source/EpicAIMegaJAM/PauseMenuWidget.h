#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class USaveSlotMenuWidget;

UCLASS()
class EPICAIMEGAJAM_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Fired when the player wants to resume. The owner closes the menu.
	FSimpleDelegate OnResumeRequested;

	// Set this in WBP_PauseMenu's Class Defaults
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Menu")
	TSubclassOf<USaveSlotMenuWidget> LoadMenuClass;

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> ResumeButton;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> LoadButton;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> QuitButton;

private:
	UFUNCTION() void HandleResume();
	UFUNCTION() void HandleLoad();
	UFUNCTION() void HandleQuit();
	void HandleLoadMenuClosed();

	UPROPERTY() TObjectPtr<USaveSlotMenuWidget> LoadMenu;
};