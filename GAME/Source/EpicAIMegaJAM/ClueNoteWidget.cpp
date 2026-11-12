#include "ClueNoteWidget.h"
#include "ClueDragDropOperation.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputCoreTypes.h"

FString UClueNoteWidget::DisplayName(const FGameplayTag& Tag)
{
	const FString Full = Tag.ToString();
	int32 Idx = INDEX_NONE;
	return Full.FindLastChar('.', Idx) ? Full.Mid(Idx + 1) : Full;
}

void UClueNoteWidget::SetClue(FGameplayTag InClue)
{
	ClueTag = InClue;
	if (NoteText)
	{
		NoteText->SetText(FText::FromString(DisplayName(InClue)));
	}
}

FReply UClueNoteWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bDraggable && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UClueNoteWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	UClueDragDropOperation* Op = NewObject<UClueDragDropOperation>();
	Op->ClueTag = ClueTag;

	// A copy of this note follows the mouse
	UClueNoteWidget* Visual = CreateWidget<UClueNoteWidget>(GetOwningPlayer(), TSubclassOf<UClueNoteWidget>(GetClass()));
	if (Visual)
	{
		Visual->bDraggable = false;
		Visual->SetClue(ClueTag);
		Op->DefaultDragVisual = Visual;
	}
	Op->Pivot = EDragPivot::MouseDown;
	OutOperation = Op;
}