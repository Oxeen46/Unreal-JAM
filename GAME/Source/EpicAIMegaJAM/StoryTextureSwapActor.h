#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "StoryTextureSwapActor.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class UStoryStateSubsystem;

// Place one in a level. While ANY of DecayFacts is true (a storyboard puzzle was solved and its reality is active),
// every static mesh that has a material override (the clean look) shows the mesh's own default material
// (the old, decayed, scary look). When the facts go away again the clean overrides are restored.
UCLASS()
class EPICAIMEGAJAM_API AStoryTextureSwapActor : public AActor
{
	GENERATED_BODY()

public:
	AStoryTextureSwapActor();

	// The scene turns old and scary while any of these facts is true (e.g. the Story.Reality.* facts)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reality")
	TArray<FGameplayTag> DecayFacts;

	// Explicit "clean material -> old material" pairs. They win over the mesh's own default material.
	// Needed for meshes whose default is an engine material (e.g. the basic Cube walls of Chapter Zero).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reality")
	TMap<TObjectPtr<UMaterialInterface>, TObjectPtr<UMaterialInterface>> ReplaceMaterials;

	// Played (2D, together) the moment the scene turns old and scary, i.e. when the reality shifts.
	// Not played when a save is loaded into an already decayed scene.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reality")
	TArray<TObjectPtr<class USoundBase>> DecaySounds;

	// How often (seconds) newly streamed-in meshes are picked up while the decayed look is active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reality")
	float RescanInterval = 2.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	struct FSlotSwap
	{
		int32 Slot = 0;
		TWeakObjectPtr<UMaterialInterface> Clean;
		TWeakObjectPtr<UMaterialInterface> Decayed;
	};

	struct FCompSwap
	{
		TWeakObjectPtr<UStaticMeshComponent> Comp;
		TArray<FSlotSwap> Slots;
	};

	UFUNCTION()
	void HandleFactChanged(FGameplayTag Fact);

	void Scan();
	void Apply();
	void Refresh();
	bool IsDecayed() const;

	TWeakObjectPtr<UStoryStateSubsystem> Story;
	TArray<FCompSwap> Entries;
	TSet<TWeakObjectPtr<UStaticMeshComponent>> Known;
	FTimerHandle RescanTimer;
	bool bDecayed = false;
	bool bRefreshedOnce = false;
};
