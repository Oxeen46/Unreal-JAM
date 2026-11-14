#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CheckpointActor.generated.h"

class UBoxComponent;
class UArrowComponent;

UCLASS()
class EPICAIMEGAJAM_API ACheckpointActor : public AActor
{
	GENERATED_BODY()

public:
	ACheckpointActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Checkpoint")
	TObjectPtr<UBoxComponent> Trigger;

	// Where the player reappears. Move and rotate this arrow in the editor.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Checkpoint")
	TObjectPtr<UArrowComponent> RespawnPoint;

	// Hook for a sound, light or effect
	UFUNCTION(BlueprintImplementableEvent, Category="Checkpoint")
	void OnCheckpointReached();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Checkpoint")
	FText PlaceName = FText::FromString(TEXT("Unknown place"));
protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool bUsed = false;
};