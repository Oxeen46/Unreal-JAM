#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemData.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Healing,
	Weapon
};

UCLASS(BlueprintType)
class EPICAIMEGAJAM_API UItemData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FGameplayTag ItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item", meta=(MultiLine=true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	EItemType Type = EItemType::Healing;

	// How many you can carry. Use 1 for weapons.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	int32 MaxStack = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Healing")
	float HealAmount = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	float Damage = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	float Range = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	float WindUpTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	float RecoveryTime = 0.6f;

	// Hits before it breaks. 0 = never breaks.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	int32 MaxDurability = 10;
};

UCLASS(BlueprintType)
class EPICAIMEGAJAM_API UItemDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Items")
	TArray<TObjectPtr<UItemData>> Items;

	UItemData* FindItem(FGameplayTag Tag) const
	{
		for (UItemData* Item : Items)
		{
			if (Item && Item->ItemTag == Tag)
			{
				return Item;
			}
		}
		return nullptr;
	}
};