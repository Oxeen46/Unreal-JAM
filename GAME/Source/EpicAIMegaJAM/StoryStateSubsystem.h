#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "StoryStateSubsystem.generated.h"

class UStoryBoardData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryFactChanged, FGameplayTag, Fact);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClueCollected, FGameplayTag, Clue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBoardChanged);

UCLASS()
class EPICAIMEGAJAM_API UStoryStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ---- Story facts (what the world reacts to) ----
	UFUNCTION(BlueprintCallable, Category="Story")
	void SetFact(FGameplayTag Fact, bool bValue);

	UFUNCTION(BlueprintPure, Category="Story")
	bool HasFact(FGameplayTag Fact) const;

	UPROPERTY(BlueprintAssignable, Category="Story")
	FOnStoryFactChanged OnFactChanged;

	// ---- Collected clues ----
	UFUNCTION(BlueprintCallable, Category="Story")
	void CollectClue(FGameplayTag Clue);

	UFUNCTION(BlueprintPure, Category="Story")
	bool HasClue(FGameplayTag Clue) const;

	UFUNCTION(BlueprintPure, Category="Story")
	TArray<FGameplayTag> GetCollectedClues() const;

	UPROPERTY(BlueprintAssignable, Category="Story")
	FOnClueCollected OnClueCollected;

	// ---- Storyboard ----
	UFUNCTION(BlueprintCallable, Category="Story")
	void PlaceClue(FGameplayTag Slot, FGameplayTag Clue);

	UFUNCTION(BlueprintCallable, Category="Story")
	void ClearSlot(FGameplayTag Slot);

	UFUNCTION(BlueprintPure, Category="Story")
	FGameplayTag GetPlacedClue(FGameplayTag Slot) const;

	// Returns true and sets the board's result facts if every slot is correct
	UFUNCTION(BlueprintCallable, Category="Story")
	bool CommitBoard(const UStoryBoardData* Board);

	UPROPERTY(BlueprintAssignable, Category="Story")
	FOnBoardChanged OnBoardChanged;

private:
	UPROPERTY()
	FGameplayTagContainer Facts;

	UPROPERTY()
	TArray<FGameplayTag> CollectedClues;

	UPROPERTY()
	TMap<FGameplayTag, FGameplayTag> PlacedClues; // slot -> clue
};