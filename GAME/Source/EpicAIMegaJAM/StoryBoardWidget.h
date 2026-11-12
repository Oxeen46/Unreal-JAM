#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "StoryBoardWidget.generated.h"

class UStoryBoardData;
class AStoryBoardStation;
class UVerticalBox;
class UHorizontalBox;
class UButton;
class UTextBlock;
class UClueNoteWidget;
class UBoardSlotWidget;

UCLASS()
class EPICAIMEGAJAM_API UStoryBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitBoard(UStoryBoardData* InBoard, AStoryBoardStation* InStation);

	// Set these in WBP_StoryBoard's Class Defaults
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Board")
	TSubclassOf<UClueNoteWidget> NoteWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Board")
	TSubclassOf<UBoardSlotWidget> SlotWidgetClass;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> SlotList;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> ClueTray;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CommitButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ResultText;

private:
	UFUNCTION()
	void HandleCommit();

	UFUNCTION()
	void HandleClose();

	UFUNCTION()
	void HandleBoardChanged();

	void RebuildAll();

	UPROPERTY()
	TObjectPtr<UStoryBoardData> Board;

	UPROPERTY()
	TObjectPtr<AStoryBoardStation> Station;
};