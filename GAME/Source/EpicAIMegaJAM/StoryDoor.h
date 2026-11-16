#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "InteractableInterface.h"
#include "StoryDoor.generated.h"


class UStaticMeshComponent;
class UStoryStateSubsystem;

UCLASS()
class EPICAIMEGAJAM_API AStoryDoor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AStoryDoor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// The door opens (disappears) when this fact becomes true
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door")
	FGameplayTag RequiredFact;

	// Hook for fancier effects later (animations, sounds, lights)
	UFUNCTION(BlueprintImplementableEvent, Category="Door")
	void OnStateChanged(bool bUnlocked);
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door")
    FText InteractionText = FText::FromString(TEXT("Try the door"));

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	

private:
	UFUNCTION()
	void HandleFactChanged(FGameplayTag Fact);

	void Apply(bool bUnlocked);

	TWeakObjectPtr<UStoryStateSubsystem> Story;
};
