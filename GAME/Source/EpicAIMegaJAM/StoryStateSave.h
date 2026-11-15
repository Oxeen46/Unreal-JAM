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
	UPROPERTY() TMap<FGameplayTag, TSoftObjectPtr<UTexture2D>> ClueIcons;
	UPROPERTY() TArray<FGameplayTag> CollectedClues;
	UPROPERTY() TMap<FGameplayTag, FText> ClueTitles;
	UPROPERTY() TMap<FGameplayTag, FText> ClueTexts;
	UPROPERTY() TMap<FString, int32> StorylineProgress;
	UPROPERTY() TMap<FString, int32> ActiveStorylines;

	UPROPERTY() TMap<FGameplayTag, int32> ItemCounts;
	UPROPERTY() TMap<FGameplayTag, int32> WeaponDurability;
	UPROPERTY() FGameplayTag EquippedWeapon;
	UPROPERTY() TArray<FString> CollectedPickups;

	UPROPERTY() bool bHasCheckpoint = false;
	UPROPERTY() FString CheckpointLevel;
	UPROPERTY() FString CheckpointName;
	UPROPERTY() FTransform CheckpointTransform;

	UPROPERTY() FString SlotLabel;
	UPROPERTY() FDateTime SavedAt;
};