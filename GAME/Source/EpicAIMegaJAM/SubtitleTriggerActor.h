#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SubtitleTriggerActor.generated.h"

class UBoxComponent;
class USoundBase;

// Plays a line when the player walks into it
UCLASS()
class EPICAIMEGAJAM_API ASubtitleTriggerActor : public AActor
{
	GENERATED_BODY()

public:
	ASubtitleTriggerActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Subtitle")
	TObjectPtr<UBoxComponent> Trigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtitle", meta=(MultiLine=true))
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtitle")
	FText Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtitle")
	TObjectPtr<USoundBase> Voice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtitle")
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtitle")
	bool bOnlyOnce = true;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool bPlayed = false;
};