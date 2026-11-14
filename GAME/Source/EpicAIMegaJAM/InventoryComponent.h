#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InventoryComponent.generated.h"

class UItemData;
class UItemDatabase;
class UInventoryWidget;
class UStoryStateSubsystem;
class APlayerController;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EPICAIMEGAJAM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// Every item in the game. Set this on the player.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UItemDatabase> Database;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;

	// Bind this to your inventory input
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void ToggleInventory();

	// Returns how many were actually added (0 if you're full)
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 AddItem(UItemData* Item, int32 Amount);

	UFUNCTION(BlueprintPure, Category="Inventory")
	UItemData* FindItem(FGameplayTag Tag) const;

	bool UseHealingItem(UItemData* Item);

	// nullptr = fists
	void EquipWeapon(UItemData* Weapon);

	// Called by combat after every successful hit
	void OnWeaponHit();

protected:
	virtual void BeginPlay() override;

private:
	void Open();
	void Close();
	void ApplyEquipped();

	UStoryStateSubsystem* GetStory() const;
	APlayerController* GetPC() const;

	UPROPERTY()
	TObjectPtr<UInventoryWidget> Widget;

	// Bare-hands stats, captured from the combat component at start
	float FistDamage = 25.f;
	float FistRange = 220.f;
	float FistWindUp = 0.25f;
	float FistRecovery = 0.45f;
	bool bFistsCaptured = false;
};