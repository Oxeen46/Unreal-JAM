#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

UCLASS()
class EPICAIMEGAJAM_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMainMenuPlayerController();

protected:
	virtual void BeginPlay() override;

	// The Widget Blueprint class you created (WBP_MainMenu)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UUserWidget> MainMenuWidgetClass;

	// Instance of the widget once created
	UPROPERTY()
	TObjectPtr<UUserWidget> MainMenuWidgetInstance;

	// Optional: function you can call later to remove the menu
	UFUNCTION(BlueprintCallable, Category = "UI")
	void RemoveMainMenu();
};