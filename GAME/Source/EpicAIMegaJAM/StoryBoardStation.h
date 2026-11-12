#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "StoryBoardStation.generated.h"

class UStaticMeshComponent;
class UStoryBoardData;
class UStoryBoardWidget;
class APlayerController;

UCLASS()
class EPICAIMEGAJAM_API AStoryBoardStation : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AStoryBoardStation();

	virtual void Interact_Implementation(AActor* Interactor) override;

	// Called by the widget
	bool TryCommit();
	void CloseBoard();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Board")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Board")
	TObjectPtr<UStoryBoardData> Board;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Board")
	TSubclassOf<UStoryBoardWidget> WidgetClass;
	
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Board")
	FText InteractionText = FText::FromString(TEXT("Open storyboard"));

private:
	UPROPERTY()
	TObjectPtr<UStoryBoardWidget> ActiveWidget;

	UPROPERTY()
	TObjectPtr<APlayerController> ActivePC;

	bool bCompleted = false;
};