#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "StoryStateSubsystem.generated.h"

class UStoryBoardData;
class UStoryStateSave;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryFactChanged, FGameplayTag, Fact);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClueCollected, FGameplayTag, Clue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBoardChanged);

USTRUCT(BlueprintType)
struct FSaveSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) bool bExists = false;
	UPROPERTY(BlueprintReadOnly) FString Label;
	UPROPERTY(BlueprintReadOnly) FString Place;
	UPROPERTY(BlueprintReadOnly) FString TimeText;
};

UCLASS()
class EPICAIMEGAJAM_API UStoryStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ---- Facts (what the world reacts to) ----
	UFUNCTION(BlueprintCallable, Category="Story")
	void SetFact(FGameplayTag Fact, bool bValue);

	UFUNCTION(BlueprintPure, Category="Story")
	bool HasFact(FGameplayTag Fact) const;

	UPROPERTY(BlueprintAssignable, Category="Story")
	FOnStoryFactChanged OnFactChanged;

	// ---- Clues ----
	UFUNCTION(BlueprintCallable, Category="Story")
	void CollectClue(FGameplayTag Clue, const FText& Title, const FText& Text);

	UFUNCTION(BlueprintPure, Category="Story")
	bool HasClue(FGameplayTag Clue) const;

	UFUNCTION(BlueprintPure, Category="Story")
	TArray<FGameplayTag> GetCollectedClues() const;

	UFUNCTION(BlueprintPure, Category="Story")
	FText GetClueTitle(FGameplayTag Clue) const;
	
	UFUNCTION(BlueprintPure, Category="Story")
	FText GetClueText(FGameplayTag Clue) const;

	UPROPERTY(BlueprintAssignable, Category="Story")
	FOnClueCollected OnClueCollected;

	// ---- Board placement ----
	UFUNCTION(BlueprintCallable, Category="Story")
	void PlaceClue(FGameplayTag Slot, FGameplayTag Clue);

	UFUNCTION(BlueprintCallable, Category="Story")
	void ClearSlot(FGameplayTag Slot);

	UFUNCTION(BlueprintPure, Category="Story")
	FGameplayTag GetPlacedClue(FGameplayTag Slot) const;

	UPROPERTY(BlueprintAssignable, Category="Story")
	FOnBoardChanged OnBoardChanged;

	// ---- Storylines ----
	// Checks the board. If it fits a storyline, confirms it, makes it the active reality
	// and returns its index. Returns -1 if nothing fits.
	UFUNCTION(BlueprintCallable, Category="Story")
	int32 CommitBoard(const UStoryBoardData* Board);

	// The currently active storyline for a board (-1 = none)
	UFUNCTION(BlueprintPure, Category="Story")
	int32 GetActiveStoryline(const UStoryBoardData* Board) const;

	// Switch reality. -1 = none. Undoes the old storyline's facts and applies the new one's.
	UFUNCTION(BlueprintCallable, Category="Story")
	void SetActiveStoryline(const UStoryBoardData* Board, int32 Index);

	UFUNCTION(BlueprintPure, Category="Story")
	TArray<int32> GetConfirmedStorylines(const UStoryBoardData* Board) const;
	
	// ---- Inventory ----
	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetItemCount(FGameplayTag Item) const;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void AddItem(FGameplayTag Item, int32 Amount = 1);

	// Returns false if you don't have enough
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveItem(FGameplayTag Item, int32 Amount = 1);

	UFUNCTION(BlueprintPure, Category="Inventory")
	FGameplayTag GetEquippedWeapon() const { return EquippedWeapon; }

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void SetEquippedWeapon(FGameplayTag Weapon) { EquippedWeapon = Weapon; }

	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetWeaponDurability(FGameplayTag Weapon) const;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void SetWeaponDurability(FGameplayTag Weapon, int32 Value);

	UFUNCTION(BlueprintPure, Category="Inventory")
	bool IsPickupCollected(const FString& PickupId) const;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void MarkPickupCollected(const FString& PickupId);
	
	// ---- Checkpoints and saving ----
	// Slot 0 is the autosave, slots 1-3 are manual
	static constexpr int32 NumSlots = 4;

	// Remembers the respawn point and the story state, and writes the autosave
	UFUNCTION(BlueprintCallable, Category="Save")
	void SetCheckpoint(const FTransform& Transform, const FText& Name);

	UFUNCTION(BlueprintPure, Category="Save")
	bool HasCheckpoint() const { return bHasCheckpoint; }

	UFUNCTION(BlueprintCallable, Category="Save")
	bool SaveToSlot(int32 SlotIndex, const FString& Label);

	UFUNCTION(BlueprintPure, Category="Save")
	FSaveSlotInfo GetSlotInfo(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category="Save")
	bool HasAnySave() const;

	// Loads a slot and travels to its checkpoint
	UFUNCTION(BlueprintCallable, Category="Save")
	bool ContinueFromSlot(int32 SlotIndex);

	// Called when the player dies
	UFUNCTION(BlueprintCallable, Category="Save")
	void RespawnFromCheckpoint();

	// Used by the player on level start. True if the player should be moved to the checkpoint.
	bool ConsumePendingRespawn(FTransform& OutTransform);

	UFUNCTION(BlueprintCallable, Category="Save")
	void ResetAllState();

private:
	UStoryStateSave* CaptureState() const;
	void ApplyState(const UStoryStateSave* Save);

	UPROPERTY() FGameplayTagContainer Facts;
	UPROPERTY() TArray<FGameplayTag> CollectedClues;
	UPROPERTY() TMap<FGameplayTag, FText> ClueTitles;
	UPROPERTY() TMap<FGameplayTag, FGameplayTag> PlacedClues;   // slot -> clue
	UPROPERTY() TMap<FString, int32> ActiveStorylines;          // board asset -> active storyline
	UPROPERTY() TArray<FString> ConfirmedStorylines;            // "BoardPath#Index"

	// Story state as it was at the last checkpoint (used when you die)
	UPROPERTY() TObjectPtr<UStoryStateSave> CheckpointSnapshot;

	bool bHasCheckpoint = false;
	bool bPendingRespawn = false;
	FString CheckpointLevel;
	FString CheckpointName;
	FTransform CheckpointTransform;
	
	UPROPERTY() TMap<FGameplayTag, FText> ClueTexts;
	UPROPERTY() TMap<FGameplayTag, int32> ItemCounts;
	UPROPERTY() TMap<FGameplayTag, int32> WeaponDurability;
	UPROPERTY() FGameplayTag EquippedWeapon;
	UPROPERTY() TArray<FString> CollectedPickups;
};