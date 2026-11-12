#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StoryBoardData.generated.h"

USTRUCT(BlueprintType)
struct FStoryBoardSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SlotTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Label;
};

USTRUCT(BlueprintType)
struct FStorylineRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SlotTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Clue;
};

USTRUCT(BlueprintType)
struct FStoryline
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	// Which clue has to sit in which slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStorylineRequirement> Requirements;

	// Facts that become true while this storyline is the chosen reality
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ResultFacts;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStoryline> Storylines;
};