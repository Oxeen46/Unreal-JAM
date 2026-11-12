#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "InteractableInterface.h"
#include "StoryBoardActor.generated.h"

class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UTextRenderComponent;
class UCameraComponent;
class UMaterialInterface;
class UStoryBoardData;
class UStoryStateSubsystem;
class APlayerController;

USTRUCT()
struct FBoardNote
{
	GENERATED_BODY()

	UPROPERTY() FGameplayTag Clue;
	UPROPERTY() TObjectPtr<USceneComponent> Root;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY() TObjectPtr<UTextRenderComponent> Text;
};

USTRUCT()
struct FBoardSlotVisual
{
	GENERATED_BODY()

	UPROPERTY() TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY() TObjectPtr<UTextRenderComponent> Text;
};

UCLASS()
class EPICAIMEGAJAM_API AStoryBoardActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AStoryBoardActor();
	
	bool bLockAfterCommit = false;

	virtual void Tick(float DeltaTime) override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Board")
	TObjectPtr<UStoryBoardData> Board;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Board")
	FText InteractionText = FText::FromString(TEXT("Open storyboard"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Board")
	float BlendTime = 0.8f;

protected:
	virtual void BeginPlay() override;

private:
	void BuildSlots();
	void BuildNotes();
	void DestroyNotes();
	void RefreshLayout();

	void EnterBoard(APlayerController* PC);
	void ExitBoard();
	void FinishExit();

	void BeginDrag();
	void UpdateDrag();
	void EndDrag();
	void TryRewrite();

	UStoryStateSubsystem* GetStory() const;

	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<UStaticMeshComponent> BoardMesh;
	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<UCameraComponent> BoardCamera;
	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<UTextRenderComponent> TitleText;
	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<UTextRenderComponent> StatusText;
	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<UTextRenderComponent> HintText;

	UPROPERTY() TObjectPtr<UStaticMesh> CubeMesh;
	UPROPERTY() TObjectPtr<UMaterialInterface> BasicMat;
	UPROPERTY() TArray<FBoardSlotVisual> SlotVisuals;
	UPROPERTY() TArray<FBoardNote> Notes;
	UPROPERTY() TObjectPtr<APlayerController> ActivePC;

	FTimerHandle ExitTimer;
	bool bActive = false;
	bool bCompleted = false;
	bool bPrevLMB = false;
	bool bPrevEnter = false;
	bool bPrevBack = false;
	int32 DraggedIndex = INDEX_NONE;
	float InputReadyTime = 0.f;
	float BoardWidth = 600.f;
};