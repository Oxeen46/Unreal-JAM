#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "InteractableInterface.h"
#include "ClueActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class EPICAIMEGAJAM_API AClueActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AClueActor();

	virtual void Interact_Implementation(AActor* Interactor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Clue")
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clue")
	TObjectPtr<UTexture2D> ClueIcon;

	// Extra sound played when this clue is collected (on top of the paper/pick-up sound), e.g. a metal can
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clue")
	TObjectPtr<class USoundBase> CollectSound;

	// Extra actors to outline when this clue is near (e.g. the paper model under a mesh-less clue).
	// Empty: the clue's own mesh is outlined, or the closest small mesh next to it.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clue")
	TArray<TObjectPtr<AActor>> HighlightActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clue")
	FGameplayTag ClueFact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clue")
	FText ClueTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clue", meta=(MultiLine=true))
	FText ClueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clue")
	bool bDestroyOnCollect = false;
	
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clue")
    FText InteractionText = FText::FromString(TEXT("Pick up"));
	
protected:
	virtual void BeginPlay() override;
};