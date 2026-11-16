#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChapterTrigger.generated.h"

class UBoxComponent;
class UChapterTitleWidget;

UCLASS()
class EPICAIMEGAJAM_API AChapterTrigger : public AActor
{
	GENERATED_BODY()

public:
	AChapterTrigger();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, Category = "Chapter")
	FText ChapterTitle;

	UPROPERTY(EditAnywhere, Category = "Chapter")
	TSubclassOf<UChapterTitleWidget> ChapterWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Chapter")
	bool bTriggerOnce = true;

	UPROPERTY(EditAnywhere, Category = "Chapter")
	float DisplayDuration = 4.0f;

	bool bTriggered = false;

	UFUNCTION()
	void OnPlayerEnter(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	FTimerHandle RemoveWidgetTimer;

	void RemoveChapterWidget();

	UPROPERTY()
	TObjectPtr<UChapterTitleWidget> ActiveWidget;
};