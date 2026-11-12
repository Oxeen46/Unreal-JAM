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
};