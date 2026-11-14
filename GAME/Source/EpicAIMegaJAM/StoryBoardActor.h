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
class UMaterialInstanceDynamic;
class UActorComponent;
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
};

USTRUCT()
struct FBoardPanel
{
	GENERATED_BODY()

	UPROPERTY() TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY() int32 StorylineIndex = INDEX_NONE;
};

USTRUCT()
struct FDimMesh
{
	GENERATED_BODY()

	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> MID;
	UPROPERTY() FLinearColor Base = FLinearColor::White;
};

USTRUCT()
struct FDimText
{
	GENERATED_BODY()

	UPROPERTY() TObjectPtr<UTextRenderComponent> Text;
	UPROPERTY() FColor Base = FColor::White;
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

	// Camera blend in and out of the board
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Board")
	float BlendTime = 0.8f;

	// How fast the camera glides between storyboards and zooms in and out
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Board")
	float ZoomSpeed = 4.f;

	// Reality change effect: a smooth dip to darkness and back. Never brighter than normal.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reality Change")
	float FlipDuration = 0.8f;

	// How dark it gets (0 = no dip, 1 = fully dark)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reality Change")
	float FlipDepth = 0.9f;

	// Slight unsteadiness in the dip. Set to 0 for a perfectly smooth fade.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reality Change")
	float FlutterAmount = 0.15f;

	// Hooks for your distortion effect, sound and so on (use them in the Blueprint)
	UFUNCTION(BlueprintImplementableEvent, Category="Reality Change")
	void OnFlipStarted();

	UFUNCTION(BlueprintImplementableEvent, Category="Reality Change")
	void OnFlipFinished();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Building
	void LayoutGrid();
	void BuildBoard();
	void BuildCluster(int32 StorylineIndex);
	void BuildTray();
	void ClearBoard();
	UStaticMeshComponent* AddCube(USceneComponent* Parent, const FVector& Loc, const FVector& Scale, const FLinearColor& Color, bool bClickable);
	UTextRenderComponent* AddText(USceneComponent* Parent, const FVector& Loc, const FText& Text, float Size, FColor Color);
	USceneComponent* AddNote(USceneComponent* Parent, const FVector& Loc, FGameplayTag Clue, int32 Order, bool bClickable);
	int32 NoteOrder(FGameplayTag Clue) const;
	bool IsRelevantClue(FGameplayTag Clue) const;
	FVector ClusterOrigin(int32 StorylineIndex) const;
	TArray<int32> RevealedList() const;
	FText PickDescription(int32 StorylineIndex) const;
	void ApplyDim(float Dim);

	// Camera and selection
	void UpdateCameraTargets();
	void UpdateHint();
	void SelectStoryline(int32 Index, bool bZoomIn);
	void StepSelection(int32 Direction);
	void SetOverview(bool bNewOverview);

	// Flow
	void EnterBoard(APlayerController* PC);
	void ExitBoard();
	void FinishExit();

	// Reality change
	void StartFlip();
	void TickFlip(float DeltaTime);
	void ToggleActive();

	// Dragging
	void BeginDrag();
	void UpdateDrag();
	void EndDrag();

	void SetStatus(const FText& Text, FColor Color);
	UStoryStateSubsystem* GetStory() const;

	UFUNCTION()
	void HandleClueCollected(FGameplayTag Clue);

	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<UStaticMeshComponent> BoardMesh;
	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<USceneComponent> ContentRoot;
	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<USceneComponent> FocusRoot;
	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<UCameraComponent> BoardCamera;
	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<UTextRenderComponent> StatusText;
	UPROPERTY(VisibleAnywhere, Category="Board") TObjectPtr<UTextRenderComponent> HintText;

	UPROPERTY() TObjectPtr<UStaticMesh> CubeMesh;
	UPROPERTY() TObjectPtr<UMaterialInterface> BasicMat;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> BoardMID;
	UPROPERTY() TObjectPtr<APlayerController> ActivePC;

	UPROPERTY() TArray<TObjectPtr<UActorComponent>> BoardComponents;
	UPROPERTY() TArray<FBoardNote> Notes;
	UPROPERTY() TArray<FBoardPanel> Panels;
	UPROPERTY() TArray<FDimMesh> DimMeshes;
	UPROPERTY() TArray<FDimText> DimTexts;

	FTimerHandle ExitTimer;

	// Layout
	int32 GridCols = 1;
	int32 GridRows = 1;
	float CellWidth = 680.f;
	float WallWidth = 800.f;
	float WallHeight = 440.f;
	float OverviewDist = 600.f;

	// State
	int32 Selected = INDEX_NONE;
	int32 DraggedIndex = INDEX_NONE;
	bool bActive = false;
	bool bOverview = false;
	float InputReadyTime = 0.f;

	FVector CamLoc = FVector(340.f, 0.f, 20.f);
	FVector TargetCamLoc = FVector(340.f, 0.f, 20.f);
	FVector FocusLoc = FVector::ZeroVector;

	// Input edges
	bool bPrevLMB = false;
	bool bPrevEnter = false;
	bool bPrevBack = false;
	bool bPrevLeft = false;
	bool bPrevRight = false;
	bool bPrevZ = false;

	// Reality change
	bool bFlipping = false;
	bool bFlipSwapped = false;
	float FlipElapsed = 0.f;
	float CurrentDim = 1.f;
};