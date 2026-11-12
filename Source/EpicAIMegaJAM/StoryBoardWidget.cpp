#include "StoryBoardWidget.h"
#include "StoryBoardData.h"
#include "StoryBoardStation.h"
#include "StoryStateSubsystem.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ComboBoxString.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

static FString ShortName(const FGameplayTag& Tag)
{
	const FString Full = Tag.ToString();
	int32 Idx = INDEX_NONE;
	return Full.FindLastChar('.', Idx) ? Full.Mid(Idx + 1) : Full;
}

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

void UStoryBoardWidget::InitBoard(UStoryBoardData* InBoard, AStoryBoardStation* InStation)
{
	Board = InBoard;
	Station = InStation;

	SlotList->ClearChildren();
	Combos.Reset();
	OptionToClue.Reset();

	UStoryStateSubsystem* Story = GetStory(this);
	if (!Board || !Story) return;

	if (TitleText) TitleText->SetText(Board->SceneTitle);
	if (ResultText) ResultText->SetText(FText::GetEmpty());

	const TArray<FGameplayTag> Clues = Story->GetCollectedClues();
	for (const FGameplayTag& Clue : Clues)
	{
		OptionToClue.Add(ShortName(Clue), Clue);
	}
	
	for (const FStoryBoardSlot& BoardSlot : Board->Slots)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
		Label->SetText(BoardSlot.Label);

		UComboBoxString* Combo = WidgetTree->ConstructWidget<UComboBoxString>();
		Combo->AddOption(TEXT("(empty)"));
		for (const FGameplayTag& Clue : Clues)
		{
			Combo->AddOption(ShortName(Clue));
		}

		const FGameplayTag Placed = Story->GetPlacedClue(BoardSlot.SlotTag);
		Combo->SetSelectedOption(Placed.IsValid() ? ShortName(Placed) : TEXT("(empty)"));

		Row->AddChildToHorizontalBox(Label);
		Row->AddChildToHorizontalBox(Combo);
		SlotList->AddChildToVerticalBox(Row);
		Combos.Add(Combo);
	}
}

void UStoryBoardWidget::HandleCommit()
{
	UStoryStateSubsystem* Story = GetStory(this);
	if (!Board || !Station || !Story) return;

	// Put whatever is selected into the slots
	for (int32 i = 0; i < Board->Slots.Num() && i < Combos.Num(); ++i)
	{
		const FGameplayTag SlotTag = Board->Slots[i].SlotTag;
		const FString Selected = Combos[i]->GetSelectedOption();

		if (const FGameplayTag* Clue = OptionToClue.Find(Selected))
		{
			Story->PlaceClue(SlotTag, *Clue);
		}
		else
		{
			Story->ClearSlot(SlotTag);
		}
	}

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