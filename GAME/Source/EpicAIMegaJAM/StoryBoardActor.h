#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "InteractableInterface.h"
#include "StoryBoardData.h"
#include "StoryBoardActor.generated.h"

class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UTextRenderComponent;
class UCameraComponent;
class UMaterialInterface;
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

USTRUCT()
struct FConfirmedBlock
{
	GENERATED_BODY()
	int32 StorylineIndex = INDEX_NONE;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY() TObjectPtr<UTextRenderComponent> Text;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Pin; // active indicator
};

UCLASS()
class EPICAIMEGAJAM_API AStoryBoardActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AStoryBoardActor();

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
	// Build
	void BuildSlots();
	void BuildNotes();
	void DestroyNotes();
	void RefreshLayout();
	void RefreshPins();

	// Flow
	void EnterBoard(APlayerController* PC);
	void ExitBoard();
	void FinishExit();

	// Drag
	void BeginDrag();
	void UpdateDrag();
	void EndDrag();

	// Logic
	void TryRewrite();
	void ActivateStoryline(int32 Index);
	int32 GetActiveStoryline() const;

	// Block row
	void AddConfirmedBlock(int32 StorylineIndex);
	void HandleBlockClick(int32 StorylineIndex);

	UStoryStateSubsystem* GetStory() const;

	// Screen fade helpers
	void StartScreenFadeOut();
	void StartScreenFadeIn();
	void OnFadeOutFinished();
	void StartScreenFadeOutForExit();
	void OnFadeOutFinishedForExit();

	// Components
	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> BoardMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> BoardCamera;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> TitleText;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> StatusText;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> HintText;

	UPROPERTY() TObjectPtr<UStaticMesh> CubeMesh;
	UPROPERTY() TObjectPtr<UMaterialInterface> BasicMat;
	UPROPERTY() TArray<FBoardSlotVisual> SlotVisuals;
	UPROPERTY() TArray<FBoardNote> Notes;
	UPROPERTY() TArray<FConfirmedBlock> ConfirmedBlocks;
	UPROPERTY() TObjectPtr<APlayerController> ActivePC;

	FTimerHandle ExitTimer;
	FTimerHandle FadeTimer;

	bool bActive = false;
	bool bPrevLMB = false;
	bool bPrevEnter = false;
	bool bPrevBack = false;
	int32 DraggedIndex = INDEX_NONE;
	float InputReadyTime = 0.f;
	float BoardWidth = 600.f;
	int32 ActiveStoryline = INDEX_NONE;

	TWeakObjectPtr<APawn> HiddenPawn;
};