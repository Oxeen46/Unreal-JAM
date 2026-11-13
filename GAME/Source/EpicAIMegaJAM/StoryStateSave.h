#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "StoryStateSave.generated.h"

UCLASS()
class EPICAIMEGAJAM_API UStoryStateSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() TArray<FGameplayTag> Facts;
	UPROPERTY() TArray<FGameplayTag> CollectedClues;
	UPROPERTY() TMap<FGameplayTag, FText> ClueTitles;
	UPROPERTY() TMap<FGameplayTag, FGameplayTag> PlacedClues;
	UPROPERTY() TMap<FString, int32> ActiveStorylines;
	UPROPERTY() TArray<FString> ConfirmedStorylines;

	UPROPERTY() bool bHasCheckpoint = false;
	UPROPERTY() FString CheckpointLevel;
	UPROPERTY() FTransform CheckpointTransform;
	
	UPROPERTY() FString CheckpointName;
	UPROPERTY() FString SlotLabel;
	UPROPERTY() FDateTime SavedAt;	
	
	UPROPERTY() TMap<FGameplayTag, FText> ClueTexts;
	UPROPERTY() TMap<FGameplayTag, int32> ItemCounts;
	UPROPERTY() TMap<FGameplayTag, int32> WeaponDurability;
	UPROPERTY() FGameplayTag EquippedWeapon;
	UPROPERTY() TArray<FString> CollectedPickups;
};