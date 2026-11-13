#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PauseMenuComponent.generated.h"

class UPauseMenuWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EPICAIMEGAJAM_API UPauseMenuComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Bind this to your pause input
	UFUNCTION(BlueprintCallable, Category="Menu")
	void TogglePauseMenu();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Menu")
	TSubclassOf<UPauseMenuWidget> PauseMenuClass;

private:
	void Open();
	void Close();

	APlayerController* GetPC() const;

	UPROPERTY()
	TObjectPtr<UPauseMenuWidget> Menu;
};