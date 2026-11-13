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

	FString ConfirmKey(const UStoryBoardData* Board, int32 Index)
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

// ---------------- Board placement ----------------

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

// ---------------- Storylines ----------------

int32 UStoryStateSubsystem::CommitBoard(const UStoryBoardData* Board)
{
	if (!Board) return INDEX_NONE;

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

	ConfirmedStorylines.AddUnique(ConfirmKey(Board, Match));
	SetActiveStoryline(Board, Match);
	return Match;
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

TArray<int32> UStoryStateSubsystem::GetConfirmedStorylines(const UStoryBoardData* Board) const
{
	TArray<int32> Result;
	if (!Board) return Result;

	const FString Prefix = Board->GetPathName() + TEXT("#");
	for (const FString& Entry : ConfirmedStorylines)
	{
		if (Entry.StartsWith(Prefix))
		{
			Result.Add(FCString::Atoi(*Entry.RightChop(Prefix.Len())));
		}
	}
	return Result;
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
	Save->PlacedClues = PlacedClues;
	Save->ActiveStorylines = ActiveStorylines;
	Save->ConfirmedStorylines = ConfirmedStorylines;
	Save->ClueTexts = ClueTexts;
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
	PlacedClues = Save->PlacedClues;
	ActiveStorylines = Save->ActiveStorylines;
	ConfirmedStorylines = Save->ConfirmedStorylines;
	ClueTexts = Save->ClueTexts;
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
	PlacedClues.Reset();
	ActiveStorylines.Reset();
	ConfirmedStorylines.Reset();
	ClueTexts.Reset();
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