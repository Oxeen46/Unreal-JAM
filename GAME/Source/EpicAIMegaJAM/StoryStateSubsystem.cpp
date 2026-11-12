#include "StoryStateSubsystem.h"
#include "StoryBoardData.h"
#include "Engine/Engine.h"

void UStoryStateSubsystem::SetFact(FGameplayTag Fact, bool bValue)
{
	if (!Fact.IsValid()) return;

	const bool bHad = Facts.HasTagExact(Fact);
	if (bValue == bHad) return;

	if (bValue) Facts.AddTag(Fact);
	else Facts.RemoveTag(Fact);

	OnFactChanged.Broadcast(Fact);
}

bool UStoryStateSubsystem::HasFact(FGameplayTag Fact) const
{
	return Facts.HasTagExact(Fact);
}

void UStoryStateSubsystem::CollectClue(FGameplayTag Clue)
{
	if (!Clue.IsValid() || CollectedClues.Contains(Clue)) return;

	CollectedClues.Add(Clue);
	OnClueCollected.Broadcast(Clue);
}

bool UStoryStateSubsystem::HasClue(FGameplayTag Clue) const
{
	return CollectedClues.Contains(Clue);
}

TArray<FGameplayTag> UStoryStateSubsystem::GetCollectedClues() const
{
	return CollectedClues;
}

void UStoryStateSubsystem::PlaceClue(FGameplayTag Slot, FGameplayTag Clue)
{
	if (!Slot.IsValid() || !HasClue(Clue)) return;

	// A clue can only sit in one slot at a time
	for (auto It = PlacedClues.CreateIterator(); It; ++It)
	{
		if (It.Value() == Clue)
		{
			It.RemoveCurrent();
		}
	}

	PlacedClues.Add(Slot, Clue);
	OnBoardChanged.Broadcast();
}

void UStoryStateSubsystem::ClearSlot(FGameplayTag Slot)
{
	if (PlacedClues.Remove(Slot) > 0)
	{
		OnBoardChanged.Broadcast();
	}
}

FGameplayTag UStoryStateSubsystem::GetPlacedClue(FGameplayTag Slot) const
{
	const FGameplayTag* Found = PlacedClues.Find(Slot);
	return Found ? *Found : FGameplayTag();
}

bool UStoryStateSubsystem::CommitBoard(const UStoryBoardData* Board)
{
	if (!Board) return false;

	for (const FStoryBoardSlot& Slot : Board->Slots)
	{
		if (GetPlacedClue(Slot.SlotTag) != Slot.CorrectClue)
		{
			return false; // wrong or empty slot, the story doesn't hold together
		}
	}

	for (const FGameplayTag& Fact : Board->ResultFacts)
	{
		SetFact(Fact, true);
	}
	return true;
}