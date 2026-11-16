#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ClueHighlightSubsystem.generated.h"

class AClueActor;
class APostProcessVolume;
class UPrimitiveComponent;
class UStoryStateSubsystem;

// Makes clues easy to spot: a soft pulsing outline (custom-depth stencil + the M_ClueOutline post process material).
// - Only while the player is near (ShowDistance) and has line of sight to the clue (no outline through walls)
// - Switches off once the clue is collected
// - Highlights the clue's own mesh, the actors listed in AClueActor::HighlightActors,
//   or (if the clue has no mesh) the closest small static mesh next to it
UCLASS()
class EPICAIMEGAJAM_API UClueHighlightSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// Outline starts to fade in at this distance (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clue Highlight")
	float ShowDistance = 600.f;

	// A mesh next to a mesh-less clue is only picked if it is within this distance (cm) ...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clue Highlight")
	float AutoPickDistance = 40.f;

	// ... and smaller than this (cm) so walls, floors and tables are never outlined
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clue Highlight")
	float AutoPickMaxSize = 150.f;

private:
	struct FEntry
	{
		TWeakObjectPtr<AClueActor> Clue;
		TArray<TWeakObjectPtr<UPrimitiveComponent>> Targets;
		FVector Center = FVector::ZeroVector;
		bool bVisible = false;
		float NextTrace = 0.f;
		int32 Level = 0;
	};

	void Rescan();
	void BuildTargets(FEntry& Entry) const;
	void SetLevel(FEntry& Entry, int32 NewLevel);

	TArray<FEntry> Entries;
	TSet<TWeakObjectPtr<AClueActor>> Known;
	float RescanTimer = 0.f;
	bool bMaterialOk = false;

	UPROPERTY()
	TObjectPtr<APostProcessVolume> Volume;
};
