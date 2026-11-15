#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WakeUpFadeActor.generated.h"

// The screen starts black and opens slowly, like waking up: the eyes open in a few heavy, half-closing blinks.
// Plays once per run (remembered in the story state, so a checkpoint reload does not black the screen out again).
UCLASS()
class EPICAIMEGAJAM_API AWakeUpFadeActor : public AActor
{
	GENERATED_BODY()

public:
	AWakeUpFadeActor();

	// Seconds the screen stays fully black at the start
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wake Up")
	float HoldBlack = 0.8f;

	// Seconds the eyes need to open completely
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wake Up")
	float OpenDuration = 6.f;

	// How many times the eyelids sag back while opening
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wake Up")
	int32 Blinks = 2;

	// 0..1, how far the screen darkens again during a blink
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wake Up")
	float BlinkDepth = 0.4f;

	// false: plays every time the level starts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wake Up")
	bool bOnlyOncePerRun = true;

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	float AlphaAt(float Elapsed) const;
	void Apply(float Alpha) const;
	FString PlayedId() const;

	float Elapsed = 0.f;
	bool bActive = false;
};
