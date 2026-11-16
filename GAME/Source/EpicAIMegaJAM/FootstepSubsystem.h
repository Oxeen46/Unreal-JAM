#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FootstepSubsystem.generated.h"

class UPrimitiveComponent;

UENUM()
enum class EFootSurface : uint8
{
	Wood,
	Carpet,
	Stone
};

// Plays a footstep for the player every stride, picked by the floor material under the feet
// (carpet / stone-tile-concrete / wood). Sounds come from SAFA-SESLER (Yurume_*).
UCLASS()
class EPICAIMEGAJAM_API UFootstepSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// Distance (cm) between two steps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footsteps")
	float StrideLength = 240.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footsteps")
	float Volume = 0.45f;

private:
	EFootSurface SurfaceFor(const UPrimitiveComponent* Floor) const;
	void PlayStep(EFootSurface Surface);

	float Travelled = 0.f;
	int32 LastIndex = -1;
};
