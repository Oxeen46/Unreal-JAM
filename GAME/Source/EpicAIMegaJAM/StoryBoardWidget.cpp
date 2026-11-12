#include "StoryBoardWidget.h"
#include "StoryBoardData.h"
#include "StoryBoardStation.h"
#include "StoryStateSubsystem.h"
#include "ClueNoteWidget.h"
#include "BoardSlotWidget.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

static UStoryStateSubsystem* GetStory(const UObject* Ctx)
{
	UWorld* World = Ctx ? Ctx->GetWorld() : nullptr;
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr;
}

void UStoryBoardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	CommitButton->OnClicked.AddDynamic(this, &UStoryBoardWidget::HandleCommit);
	CloseButton->OnClicked.AddDynamic(this, &UStoryBoardWidget::HandleClose);
}

void UStoryBoardWidget::NativeDestruct()
{
	if (UStoryStateSubsystem* Story = GetStory(this))
	{
		Story->OnBoardChanged.RemoveDynamic(this, &UStoryBoardWidget::HandleBoardChanged);
	}
	Super::NativeDestruct();
}

void UStoryBoardWidget::InitBoard(UStoryBoardData* InBoard, AStoryBoardStation* InStation)
{
	Board = InBoard;
	Station = InStation;

	if (TitleText && Board) TitleText->SetText(Board->SceneTitle);
	if (ResultText) ResultText->SetText(FText::GetEmpty());

	if (UStoryStateSubsystem* Story = GetStory(this))
	{
		Story->OnBoardChanged.AddUniqueDynamic(this, &UStoryBoardWidget::HandleBoardChanged);
	}

	RebuildAll();
}

void UStoryBoardWidget::HandleBoardChanged()
{
	// Wait one tick so we don't rebuild widgets in the middle of a drop
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &UStoryBoardWidget::RebuildAll);
	}
}

void UStoryBoardWidget::RebuildAll()
{
	UStoryStateSubsystem* Story = GetStory(this);
	if (!Board || !Story || !NoteWidgetClass || !SlotWidgetClass) return;

	SlotList->ClearChildren();
	ClueTray->ClearChildren();

	TSet<FGameplayTag> Placed;

	for (const FStoryBoardSlot& BoardSlot : Board->Slots)
	{
		UBoardSlotWidget* SlotWidget = CreateWidget<UBoardSlotWidget>(this, SlotWidgetClass);
		if (!SlotWidget) continue;

		const FGameplayTag PlacedClue = Story->GetPlacedClue(BoardSlot.SlotTag);
		SlotWidget->InitSlot(BoardSlot.SlotTag, BoardSlot.Label, PlacedClue);
		SlotList->AddChildToVerticalBox(SlotWidget);

		if (PlacedClue.IsValid()) Placed.Add(PlacedClue);
	}

	for (const FGameplayTag& Clue : Story->GetCollectedClues())
	{
		if (Placed.Contains(Clue)) continue;

		UClueNoteWidget* Note = CreateWidget<UClueNoteWidget>(this, NoteWidgetClass);
		if (!Note) continue;

		Note->SetClue(Clue);
		ClueTray->AddChildToHorizontalBox(Note);
	}
}

void UStoryBoardWidget::HandleCommit()
{
	if (!Station) return;

	if (Station->TryCommit())
	{
		Station->CloseBoard();
	}
	else if (ResultText)
	{
		ResultText->SetText(FText::FromString(TEXT("That doesn't fit.")));
	}
}

void UStoryBoardWidget::HandleClose()
{
	if (Station) Station->CloseBoard();
}