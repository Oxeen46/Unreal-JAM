#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StoryBoardData.generated.h"

USTRUCT(BlueprintType)
struct FStoryBoardSlot
{
	GENERATED_BODY()

	// Identifies this slot, e.g. Story.Slot.Motive
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SlotTag;

	// The clue that belongs here
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag CorrectClue;

	// Text shown on the board
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Label;
};

UCLASS(BlueprintType)
class EPICAIMEGAJAM_API UStoryBoardData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SceneTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStoryBoardSlot> Slots;

	// Facts set to true when the board is committed correctly
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ResultFacts;
};