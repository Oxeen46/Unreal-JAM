#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StoryBoardData.generated.h"

USTRUCT(BlueprintType)
struct FStoryNote
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Clue;

	// Shown under the title while this is the latest clue you FOUND for the storyboard (not placed yet)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true))
	FText FoundDescription;

	// Shown while this is the last note you PLACED
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true))
	FText PlacedDescription;
};

USTRUCT(BlueprintType)
struct FStoryline
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	// The notes in story order. The first entry is note #1.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStoryNote> Notes;

	// Shown once the storyboard is complete
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true))
	FText CompletedDescription;

	// Facts that become true while this is the active reality
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ResultFacts;

	// Visible on the board before any clue is found (leave off for most)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRevealedFromStart = false;

	int32 IndexOfClue(const FGameplayTag& Clue) const
	{
		for (int32 i = 0; i < Notes.Num(); ++i)
		{
			if (Notes[i].Clue == Clue) return i;
		}
		return INDEX_NONE;
	}
};

UCLASS(BlueprintType)
class EPICAIMEGAJAM_API UStoryBoardData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SceneTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStoryline> Storylines;

	// Clues that show up in the tray but belong to no storyline (red herrings)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGameplayTag> DecoyClues;
};