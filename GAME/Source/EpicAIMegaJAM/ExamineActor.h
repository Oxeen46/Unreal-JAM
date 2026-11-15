#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "ExamineActor.generated.h"

class UStaticMeshComponent;
class USoundBase;

// Any prop the player can press E on to read a line: "It won't open.", "A dusty mirror."
UCLASS()
class EPICAIMEGAJAM_API AExamineActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AExamineActor();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Examine")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine")
	FText InteractionText = FText::FromString(TEXT("Examine"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine", meta=(MultiLine=true))
	FText Message;

	// Leave empty (and no Voice) for a plain message
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine")
	FText Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine")
	TObjectPtr<USoundBase> Voice;

	// 0 = automatic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine")
	float Duration = 0.f;
};