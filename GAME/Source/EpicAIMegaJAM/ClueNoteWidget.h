#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "ClueNoteWidget.generated.h"

class UTextBlock;

UCLASS()
class EPICAIMEGAJAM_API UClueNoteWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetClue(FGameplayTag InClue);

	// Last part of the tag, e.g. Story.Clue.BrokenKey -> BrokenKey
	static FString DisplayName(const FGameplayTag& Tag);

	// False for the copy that follows the mouse while dragging
	bool bDraggable = true;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NoteText;

private:
	FGameplayTag ClueTag;
};