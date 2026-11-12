#include "BoardSlotWidget.h"
#include "ClueDragDropOperation.h"
#include "ClueNoteWidget.h"
#include "StoryStateSubsystem.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"

static UStoryStateSubsystem* GetStoryForSlot(const UObject* Ctx)
{
	UWorld* World = Ctx ? Ctx->GetWorld() : nullptr;
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr;
}

void UBoardSlotWidget::InitSlot(FGameplayTag InSlotTag, const FText& InLabel, FGameplayTag PlacedClue)
{
	SlotTag = InSlotTag;
	LabelText->SetText(InLabel);
	ClueText->SetText(PlacedClue.IsValid()
		? FText::FromString(UClueNoteWidget::DisplayName(PlacedClue))
		: FText::FromString(TEXT("Drop a note here")));
}

bool UBoardSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (UClueDragDropOperation* Op = Cast<UClueDragDropOperation>(InOperation))
	{
		if (UStoryStateSubsystem* Story = GetStoryForSlot(this))
		{
			Story->PlaceClue(SlotTag, Op->ClueTag);
			return true;
		}
	}
	return false;
}

FReply UBoardSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (UStoryStateSubsystem* Story = GetStoryForSlot(this))
		{
			Story->ClearSlot(SlotTag);
		}
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}