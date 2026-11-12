#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoryBoardWidget.generated.h"

class UStoryBoardData;
class AStoryBoardStation;
class UVerticalBox;
class UButton;
class UTextBlock;
class UComboBoxString;

UCLASS()
class EPICAIMEGAJAM_API UStoryBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitBoard(UStoryBoardData* InBoard, AStoryBoardStation* InStation);

protected:
	virtual void NativeOnInitialized() override;

	// These names must match the widgets you add in the editor
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> SlotList;

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

	UPROPERTY()
	TObjectPtr<UStoryBoardData> Board;

	UPROPERTY()
	TObjectPtr<AStoryBoardStation> Station;

	UPROPERTY()
	TArray<TObjectPtr<UComboBoxString>> Combos; // same order as Board->Slots

	TMap<FString, FGameplayTag> OptionToClue;
};