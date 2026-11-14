#include "StoryStateSubsystem.h"
#include "StoryBoardData.h"
#include "StoryStateSave.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

namespace
{
	FString SlotName(int32 Index)
	{
		return FString::Printf(TEXT("Slot_%d"), Index);
	}

	FString ProgressKey(const UStoryBoardData* Board, int32 Index)
	{
		return FString::Printf(TEXT("%s#%d"), *Board->GetPathName(), Index);
	}
}

// ---------------- Facts ----------------

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

// ---------------- Clues ----------------

void UStoryStateSubsystem::CollectClue(FGameplayTag Clue, const FText& Title, const FText& Text)
{
	if (!Clue.IsValid() || CollectedClues.Contains(Clue)) return;

	CollectedClues.Add(Clue);
	ClueTitles.Add(Clue, Title);
	ClueTexts.Add(Clue, Text);
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

FText UStoryStateSubsystem::GetClueTitle(FGameplayTag Clue) const
{
	const FText* Found = ClueTitles.Find(Clue);
	return Found ? *Found : FText::GetEmpty();
}

FText UStoryStateSubsystem::GetClueText(FGameplayTag Clue) const
{
	const FText* Found = ClueTexts.Find(Clue);
	return Found ? *Found : FText::GetEmpty();
}

// ---------------- Storyboard ----------------

int32 UStoryStateSubsystem::GetStorylineProgress(const UStoryBoardData* Board, int32 StorylineIndex) const
{
	if (!Board) return 0;
	const int32* Found = StorylineProgress.Find(ProgressKey(Board, StorylineIndex));
	return Found ? *Found : 0;
}

bool UStoryStateSubsystem::IsStorylineUnlocked(const UStoryBoardData* Board, int32 StorylineIndex) const
{
	if (!Board || !Board->Storylines.IsValidIndex(StorylineIndex)) return false;
	const int32 Length = Board->Storylines[StorylineIndex].Notes.Num();
	return Length > 0 && GetStorylineProgress(Board, StorylineIndex) >= Length;
}

bool UStoryStateSubsystem::IsStorylineRevealed(const UStoryBoardData* Board, int32 StorylineIndex) const
{
	if (!Board || !Board->Storylines.IsValidIndex(StorylineIndex)) return false;

	const FStoryline& Line = Board->Storylines[StorylineIndex];
	if (Line.bRevealedFromStart) return true;

	for (const FStoryNote& Note : Line.Notes)
	{
		if (CollectedClues.Contains(Note.Clue)) return true;
	}
	return false;
}

bool UStoryStateSubsystem::TryPlaceNote(const UStoryBoardData* Board, int32 StorylineIndex, int32 SlotIndex, FGameplayTag Clue)
{
	if (!Board || !Board->Storylines.IsValidIndex(StorylineIndex) || !HasClue(Clue)) return false;

	const FStoryline& Line = Board->Storylines[StorylineIndex];
	const int32 Progress = GetStorylineProgress(Board, StorylineIndex);

	// Must be the next empty slot, and the right note for it
	if (SlotIndex != Progress || !Line.Notes.IsValidIndex(Progress) || Line.Notes[Progress].Clue != Clue) return false;

	StorylineProgress.Add(ProgressKey(Board, StorylineIndex), Progress + 1);

	if (Progress + 1 >= Line.Notes.Num())
	{
		SetActiveStoryline(Board, StorylineIndex); // completing a storyboard shifts reality
	}
	return true;
}

bool UStoryStateSubsystem::IsClueConsumed(const UStoryBoardData* Board, FGameplayTag Clue) const
{
	if (!Board) return false;

	for (int32 i = 0; i < Board->Storylines.Num(); ++i)
	{
		const int32 Position = Board->Storylines[i].IndexOfClue(Clue);
		if (Position != INDEX_NONE && GetStorylineProgress(Board, i) > Position)
		{
			return true;
		}
	}
	return false;
}


FGameplayTag UStoryStateSubsystem::GetLastCollectedClueFor(const UStoryBoardData* Board, int32 StorylineIndex) const
{
	if (!Board || !Board->Storylines.IsValidIndex(StorylineIndex)) return FGameplayTag();

	const FStoryline& Line = Board->Storylines[StorylineIndex];
	for (int32 i = CollectedClues.Num() - 1; i >= 0; --i)
	{
		if (Line.IndexOfClue(CollectedClues[i]) != INDEX_NONE)
		{
			return CollectedClues[i];
		}
	}
	return FGameplayTag();
}

int32 UStoryStateSubsystem::GetActiveStoryline(const UStoryBoardData* Board) const
{
	if (!Board) return INDEX_NONE;
	const int32* Found = ActiveStorylines.Find(Board->GetPathName());
	return Found ? *Found : INDEX_NONE;
}

void UStoryStateSubsystem::SetActiveStoryline(const UStoryBoardData* Board, int32 Index)
{
	if (!Board) return;

	const FString Key = Board->GetPathName();
	const int32 Previous = GetActiveStoryline(Board);
	if (Previous == Index) return;

	FGameplayTagContainer NewFacts;
	if (Board->Storylines.IsValidIndex(Index))
	{
		NewFacts = Board->Storylines[Index].ResultFacts;
	}

	// Undo the old reality (except facts the new one also sets)
	if (Board->Storylines.IsValidIndex(Previous))
	{
		for (const FGameplayTag& Old : Board->Storylines[Previous].ResultFacts)
		{
			if (!NewFacts.HasTagExact(Old))
			{
				SetFact(Old, false);
			}
		}
	}

	for (const FGameplayTag& Fact : NewFacts)
	{
		SetFact(Fact, true);
	}

	if (Board->Storylines.IsValidIndex(Index)) ActiveStorylines.Add(Key, Index);
	else ActiveStorylines.Remove(Key);
}

// ---------------- Inventory ----------------

int32 UStoryStateSubsystem::GetItemCount(FGameplayTag Item) const
{
	const int32* Found = ItemCounts.Find(Item);
	return Found ? *Found : 0;
}

void UStoryStateSubsystem::AddItem(FGameplayTag Item, int32 Amount)
{
	if (!Item.IsValid() || Amount <= 0) return;
	ItemCounts.FindOrAdd(Item) += Amount;
}

bool UStoryStateSubsystem::RemoveItem(FGameplayTag Item, int32 Amount)
{
	int32* Found = ItemCounts.Find(Item);
	if (!Found || Amount <= 0 || *Found < Amount) return false;

	*Found -= Amount;
	if (*Found <= 0)
	{
		ItemCounts.Remove(Item);
	}
	return true;
}

int32 UStoryStateSubsystem::GetWeaponDurability(FGameplayTag Weapon) const
{
	const int32* Found = WeaponDurability.Find(Weapon);
	return Found ? *Found : 0;
}

void UStoryStateSubsystem::SetWeaponDurability(FGameplayTag Weapon, int32 Value)
{
	WeaponDurability.Add(Weapon, Value);
}

bool UStoryStateSubsystem::IsPickupCollected(const FString& PickupId) const
{
	return CollectedPickups.Contains(PickupId);
}

void UStoryStateSubsystem::MarkPickupCollected(const FString& PickupId)
{
	CollectedPickups.AddUnique(PickupId);
}

// ---------------- Checkpoints and saving ----------------

UStoryStateSave* UStoryStateSubsystem::CaptureState() const
{
	UStoryStateSave* Save = Cast<UStoryStateSave>(
		UGameplayStatics::CreateSaveGameObject(UStoryStateSave::StaticClass()));
	if (!Save) return nullptr;

	Facts.GetGameplayTagArray(Save->Facts);
	Save->CollectedClues = CollectedClues;
	Save->ClueTitles = ClueTitles;
	Save->ClueTexts = ClueTexts;
	Save->StorylineProgress = StorylineProgress;
	Save->ActiveStorylines = ActiveStorylines;
	Save->ItemCounts = ItemCounts;
	Save->WeaponDurability = WeaponDurability;
	Save->EquippedWeapon = EquippedWeapon;
	Save->CollectedPickups = CollectedPickups;
	Save->bHasCheckpoint = bHasCheckpoint;
	Save->CheckpointLevel = CheckpointLevel;
	Save->CheckpointName = CheckpointName;
	Save->CheckpointTransform = CheckpointTransform;
	return Save;
}

void UStoryStateSubsystem::ApplyState(const UStoryStateSave* Save)
{
	if (!Save) return;

	// Restored silently. The level is reloaded right after, so actors read the new state.
	Facts = FGameplayTagContainer::CreateFromArray(Save->Facts);
	CollectedClues = Save->CollectedClues;
	ClueTitles = Save->ClueTitles;
	ClueTexts = Save->ClueTexts;
	StorylineProgress = Save->StorylineProgress;
	ActiveStorylines = Save->ActiveStorylines;
	ItemCounts = Save->ItemCounts;
	WeaponDurability = Save->WeaponDurability;
	EquippedWeapon = Save->EquippedWeapon;
	CollectedPickups = Save->CollectedPickups;
	bHasCheckpoint = Save->bHasCheckpoint;
	CheckpointLevel = Save->CheckpointLevel;
	CheckpointName = Save->CheckpointName;
	CheckpointTransform = Save->CheckpointTransform;
}

void UStoryStateSubsystem::SetCheckpoint(const FTransform& Transform, const FText& Name)
{
	bHasCheckpoint = true;
	CheckpointTransform = Transform;
	CheckpointName = Name.ToString();
	if (UWorld* World = GetWorld())
	{
		CheckpointLevel = UGameplayStatics::GetCurrentLevelName(World, true);
	}

	CheckpointSnapshot = CaptureState();
	SaveToSlot(0, TEXT("Autosave"));
}

bool UStoryStateSubsystem::SaveToSlot(int32 SlotIndex, const FString& Label)
{
	if (SlotIndex < 0 || SlotIndex >= NumSlots) return false;

	UStoryStateSave* Save = CaptureState();
	if (!Save) return false;

	Save->SlotLabel = Label;
	Save->SavedAt = FDateTime::Now();
	return UGameplayStatics::SaveGameToSlot(Save, SlotName(SlotIndex), 0);
}

FSaveSlotInfo UStoryStateSubsystem::GetSlotInfo(int32 SlotIndex) const
{
	FSaveSlotInfo Info;
	if (SlotIndex < 0 || SlotIndex >= NumSlots) return Info;
	if (!UGameplayStatics::DoesSaveGameExist(SlotName(SlotIndex), 0)) return Info;

	const UStoryStateSave* Save = Cast<UStoryStateSave>(
		UGameplayStatics::LoadGameFromSlot(SlotName(SlotIndex), 0));
	if (!Save) return Info;

	Info.bExists = true;
	Info.Label = Save->SlotLabel;
	Info.Place = Save->CheckpointName.IsEmpty() ? Save->CheckpointLevel : Save->CheckpointName;
	Info.TimeText = Save->SavedAt.ToString(TEXT("%Y-%m-%d %H:%M"));
	return Info;
}

bool UStoryStateSubsystem::HasAnySave() const
{
	for (int32 i = 0; i < NumSlots; ++i)
	{
		if (UGameplayStatics::DoesSaveGameExist(SlotName(i), 0)) return true;
	}
	return false;
}

bool UStoryStateSubsystem::ContinueFromSlot(int32 SlotIndex)
{
	UWorld* World = GetWorld();
	if (!World || SlotIndex < 0 || SlotIndex >= NumSlots) return false;
	if (!UGameplayStatics::DoesSaveGameExist(SlotName(SlotIndex), 0)) return false;

	UStoryStateSave* Save = Cast<UStoryStateSave>(
		UGameplayStatics::LoadGameFromSlot(SlotName(SlotIndex), 0));
	if (!Save || !Save->bHasCheckpoint || Save->CheckpointLevel.IsEmpty()) return false;

	ApplyState(Save);
	CheckpointSnapshot = Save; // dying after loading respawns here
	bPendingRespawn = true;
	UGameplayStatics::OpenLevel(World, FName(*CheckpointLevel));
	return true;
}

void UStoryStateSubsystem::RespawnFromCheckpoint()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (CheckpointSnapshot)
	{
		ApplyState(CheckpointSnapshot);
		bPendingRespawn = true;
		UGameplayStatics::OpenLevel(World, FName(*CheckpointLevel));
		return;
	}

	// No checkpoint yet: restart the level from scratch
	ResetAllState();
	UGameplayStatics::OpenLevel(World, FName(*UGameplayStatics::GetCurrentLevelName(World, true)));
}

bool UStoryStateSubsystem::ConsumePendingRespawn(FTransform& OutTransform)
{
	if (!bPendingRespawn) return false;
	bPendingRespawn = false;
	OutTransform = CheckpointTransform;
	return true;
}

void UStoryStateSubsystem::ResetAllState()
{
	Facts.Reset();
	CollectedClues.Reset();
	ClueTitles.Reset();
	ClueTexts.Reset();
	StorylineProgress.Reset();
	ActiveStorylines.Reset();
	ItemCounts.Reset();
	WeaponDurability.Reset();
	EquippedWeapon = FGameplayTag();
	CollectedPickups.Reset();
	CheckpointSnapshot = nullptr;
	bHasCheckpoint = false;
	bPendingRespawn = false;
	CheckpointLevel.Empty();
	CheckpointName.Empty();
	CheckpointTransform = FTransform::Identity;
}