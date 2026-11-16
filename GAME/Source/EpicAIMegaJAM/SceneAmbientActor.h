#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "SceneAudioActor.h"
#include "SceneAmbientActor.generated.h"

class UAudioComponent;
class USoundBase;
class USoundAttenuation;
class UStoryStateSubsystem;

// A sound bed for a level: optional intro sounds (played once, one after another), then a sound that loops.
// It can wait for a story fact before it starts and fade out when another fact becomes true,
// e.g. the TV in Joe's living room plays until the reality shifts.
UCLASS()
class EPICAIMEGAJAM_API ASceneAmbientActor : public AActor
{
	GENERATED_BODY()

public:
	ASceneAmbientActor();

	// Played once, in order, before the loop starts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient")
	TArray<TObjectPtr<USoundBase>> IntroSounds;

	// Repeats until the bed is stopped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient")
	TObjectPtr<USoundBase> LoopSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient")
	float Volume = 1.f;

	// Sfx: follows the SFX slider of the options menu. Music: follows the Music slider.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient")
	ESceneAudioChannel Channel = ESceneAudioChannel::Sfx;

	// true: comes from this actor's position (uses Attenuation). false: plain 2D (music)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient")
	bool bSpatial = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient", meta=(EditCondition="bSpatial"))
	TObjectPtr<USoundAttenuation> Attenuation;

	// Seconds after the level begins (or after StartWhenFact became true)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient")
	float StartDelay = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient")
	float FadeInTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient")
	float FadeOutTime = 2.f;

	// Pause (seconds) between two repeats of the loop sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient")
	float RepeatGap = 0.f;

	// Only runs while this fact is true (leave empty: runs from the start)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient")
	FGameplayTag StartWhenFact;

	// Fades out and stays off once this fact is true (leave empty: never stops)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambient")
	FGameplayTag StopWhenFact;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleFactChanged(FGameplayTag Fact);

	UFUNCTION()
	void HandleFinished();

	void Evaluate();
	void BeginRun();
	void StopRun();
	void PlayNext();

	UPROPERTY()
	TObjectPtr<UAudioComponent> Audio;

	TWeakObjectPtr<UStoryStateSubsystem> Story;
	FTimerHandle StartTimer;
	FTimerHandle NextTimer;
	int32 IntroIndex = 0;
	bool bRunning = false;
	bool bStartPending = false;
};
