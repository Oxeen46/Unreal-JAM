#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "MainMenuWidget.generated.h"

UCLASS()
class EPICAIMEGAJAM_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// Buttons (bind these in the Blueprint widget)
	UPROPERTY(meta = (BindWidget))
	UButton* StartButton;

	UPROPERTY(meta = (BindWidget))
	UButton* OptionsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;
	
	// The name of your gameplay level, set in WBP_MainMenu's Class Defaults
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Menu")
	FName GameLevelName;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UOptionsMenuWidget> OptionsMenuClass;

	// Button callbacks
	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnOptionsClicked();
	
	UFUNCTION()
	void HandleOptionsBack();

	UPROPERTY()
	TObjectPtr<UOptionsMenuWidget> OptionsMenu;

	UFUNCTION()
	void OnQuitClicked();
	
};