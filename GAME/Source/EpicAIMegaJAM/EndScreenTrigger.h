#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EndScreenTrigger.generated.h"

class UBoxComponent;
class UEndScreenWidget;

// When the player walks into this box the screen fades to black with a closing message
// ("Thank you for playing"). It does NOT load another level.
UCLASS()
class EPICAIMEGAJAM_API AEndScreenTrigger : public AActor
{
	GENERATED_BODY()

public:
	AEndScreenTrigger();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="End Screen")
	TObjectPtr<UBoxComponent> Trigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="End Screen")
	FText Message = FText::FromString(TEXT("Thank you for playing"));

	// Seconds the card needs to fade in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="End Screen")
	float FadeTime = 2.5f;

	// Stop the player from walking on once the card shows (looking around and the pause menu still work)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="End Screen")
	bool bStopMovement = true;

	// After the card has faded in and stayed for HoldTime seconds the game goes back to the main menu
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="End Screen")
	bool bReturnToMenu = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="End Screen", meta=(EditCondition="bReturnToMenu"))
	float HoldTime = 5.f;

	// Level to open at the end (full path or short name)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="End Screen", meta=(EditCondition="bReturnToMenu"))
	FName MenuLevel = FName(TEXT("/Game/ModeLM/Maps/Lvl_MainMenu"));

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void GoToMenu();

	UPROPERTY()
	TObjectPtr<UEndScreenWidget> Widget;

	FTimerHandle MenuTimer;

	bool bShown = false;
};
