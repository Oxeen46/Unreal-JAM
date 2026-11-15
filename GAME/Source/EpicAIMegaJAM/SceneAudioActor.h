#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SceneAudioActor.generated.h"

class USoundBase;
class USoundAttenuation;

// Which volume slider in the main menu options controls the sound
UENUM(BlueprintType)
enum class ESceneAudioChannel : uint8
{
	Sfx,
	Music
};

UENUM(BlueprintType)
enum class ESceneAudioTrigger : uint8
{
	// Starts when the player comes within TriggerDistance of ProximityTarget
	OnApproach,
	// Starts StartDelay seconds after the level begins
	AfterSeconds
};

USTRUCT(BlueprintType)
struct FSceneAudioStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Volume = 1.f;

	// Silence (seconds) after this sound finishes, before the next step starts
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapAfter = 0.3f;
};

// A one-shot sound sequence for a level. Plays its steps one after another, exactly once per run
// (remembered in the story state, so loading a checkpoint does not replay it).
UCLASS()
class EPICAIMEGAJAM_API ASceneAudioActor : public AActor
{
	GENERATED_BODY()

public:
	ASceneAudioActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scene Audio")
	ESceneAudioTrigger Trigger = ESceneAudioTrigger::AfterSeconds;

	// Played in this order, each after the previous one has finished
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scene Audio")
	TArray<FSceneAudioStep> Steps;

	// OnApproach: the object the player has to come near (e.g. the stairs). Empty = this actor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scene Audio")
	TObjectPtr<AActor> ProximityTarget;

	// OnApproach: distance (cm) from the target's bounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scene Audio")
	float TriggerDistance = 200.f;

	// AfterSeconds: seconds after the level begins
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scene Audio")
	float StartDelay = 6.f;

	// true: the sound comes from this actor's position in the world (uses Attenuation). false: plain 2D sound.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scene Audio")
	bool bSpatial = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scene Audio", meta=(EditCondition="bSpatial"))
	TObjectPtr<USoundAttenuation> Attenuation;

	// Sfx: follows the SFX slider of the options menu. Music: follows the Music slider.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scene Audio")
	ESceneAudioChannel Channel = ESceneAudioChannel::Sfx;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void CheckApproach();
	void StartSequence();
	void PlayStep();
	FString PlayedId() const;

	UPROPERTY()
	TObjectPtr<class UAudioComponent> Audio;

	FTimerHandle CheckTimer;
	FTimerHandle StartTimer;
	FTimerHandle StepTimer;
	int32 StepIndex = 0;
	bool bPlayed = false;
};
