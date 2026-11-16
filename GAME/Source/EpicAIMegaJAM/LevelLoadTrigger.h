#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelLoadTrigger.generated.h"

class UBoxComponent;
class UChapterTitleWidget;

UCLASS()
class EPICAIMEGAJAM_API ALevelLoadTrigger : public AActor
{
	GENERATED_BODY()

public:
	ALevelLoadTrigger();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, Category = "Level")
	FName LevelToLoad;

	UPROPERTY(EditAnywhere, Category = "Chapter")
	FText ChapterTitle;

	UPROPERTY(EditAnywhere, Category = "Chapter")
	TSubclassOf<UChapterTitleWidget> ChapterWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Chapter")
	float DisplayDuration = 4.0f;

	bool bTriggered = false;

	FTimerHandle LoadLevelTimer;

	UFUNCTION()
	void OnPlayerEnter(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void LoadNextLevel();
};