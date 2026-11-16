#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UInventoryRowWidget;
class UVerticalBox;
class UButton;
class UTextBlock;
class UImage;
class UPanelWidget;
class UTexture2D;

UENUM()
enum class EInventoryTab : uint8
{
	Notes,
	Healing,
	Weapons
};

UCLASS()
class EPICAIMEGAJAM_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(UInventoryComponent* InInventory);

	FSimpleDelegate OnCloseRequested;

	// Set in WBP_Inventory's Class Defaults
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TSubclassOf<UInventoryRowWidget> RowClass;

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> NotesTabButton;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> HealingTabButton;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> WeaponsTabButton;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> CloseButton;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UPanelWidget> ItemList;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UImage> DetailIcon;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> DetailTitle;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> DetailBody;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> ActionButton;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> ActionText;

	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> TabTitleText;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> HealthText;

private:
	struct FEntry
	{
		FGameplayTag Tag;
		FText Label;
		FText Title;
		FText Body;
		FText ActionLabel; // empty = no action button
		bool bCanAct = false;
	};
	UTexture2D* EntryIcon(const FEntry& Entry) const;

	UFUNCTION() void HandleNotesTab();
	UFUNCTION() void HandleHealingTab();
	UFUNCTION() void HandleWeaponsTab();
	UFUNCTION() void HandleClose();
	UFUNCTION() void HandleAction();

	void HandleRowClicked(int32 Index);
	void SetTab(EInventoryTab Tab);
	void Rebuild();
	void BuildEntries();
	void UpdateDetails();
	void UpdateHeader();

	UPROPERTY()
	TObjectPtr<UInventoryComponent> Inventory;

	TArray<FEntry> Entries;
	EInventoryTab CurrentTab = EInventoryTab::Notes;
	int32 Selected = 0;
};