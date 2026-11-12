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

void UStoryStateSubsystem::CollectClue(FGameplayTag Clue, const FText& Title)
{
	if (!Clue.IsValid() || CollectedClues.Contains(Clue)) return;

	CollectedClues.Add(Clue);
	ClueTitles.Add(Clue, Title);
	OnClueCollected.Broadcast(Clue);
}

FText UStoryStateSubsystem::GetClueTitle(FGameplayTag Clue) const
{
	const FText* Found = ClueTitles.Find(Clue);
	return Found ? *Found : FText::GetEmpty();
}

int32 UStoryStateSubsystem::GetCommittedStoryline(const UStoryBoardData* Board) const
{
	if (!Board) return INDEX_NONE;
	const int32* Found = CommittedStorylines.Find(Board->GetPathName());
	return Found ? *Found : INDEX_NONE;
}

int32 UStoryStateSubsystem::CommitBoard(const UStoryBoardData* Board)
{
	if (!Board) return INDEX_NONE;

	// Find the storyline that matches what is on the board
	int32 Match = INDEX_NONE;
	for (int32 i = 0; i < Board->Storylines.Num() && Match == INDEX_NONE; ++i)
	{
		bool bAllMatch = Board->Storylines[i].Requirements.Num() > 0;
		for (const FStorylineRequirement& Req : Board->Storylines[i].Requirements)
		{
			if (GetPlacedClue(Req.SlotTag) != Req.Clue)
			{
				bAllMatch = false;
				break;
			}
		}
		if (bAllMatch) Match = i;
	}
	if (Match == INDEX_NONE) return INDEX_NONE;

	const FGameplayTagContainer& NewFacts = Board->Storylines[Match].ResultFacts;

	// Undo facts from the previously chosen storyline (the ones the new one doesn't also set)
	const FString Key = Board->GetPathName();
	if (const int32* Previous = CommittedStorylines.Find(Key))
	{
		if (Board->Storylines.IsValidIndex(*Previous))
		{
			for (const FGameplayTag& Old : Board->Storylines[*Previous].ResultFacts)
			{
				if (!NewFacts.HasTagExact(Old))
				{
					SetFact(Old, false);
				}
			}
		}
	}

	for (const FGameplayTag& Fact : NewFacts)
	{
		SetFact(Fact, true);
	}

	CommittedStorylines.Add(Key, Match);
	return Match;
}

void UStoryStateSubsystem::RemoveCollectedClue(FGameplayTag Clue)
{
	CollectedClues.Remove(Clue);          // or whatever your container is called
	// also clear any placement that still references it, just in case
	for (auto& Pair : PlacedClues)
	{
		if (Pair.Value == Clue)
			Pair.Value = FGameplayTag();
	}
}