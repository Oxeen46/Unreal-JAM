#include "InventoryWidget.h"
#include "InventoryComponent.h"
#include "InventoryRowWidget.h"
#include "ItemData.h"
#include "PlayerCombatComponent.h"
#include "StoryStateSubsystem.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"

namespace
{
	UStoryStateSubsystem* InvUI_GetStory(const UObject* Ctx)
	{
		UWorld* World = Ctx ? Ctx->GetWorld() : nullptr;
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr;
	}

	FString InvUI_ShortName(const FGameplayTag& Tag)
	{
		const FString Full = Tag.ToString();
		int32 Idx = INDEX_NONE;
		return Full.FindLastChar('.', Idx) ? Full.Mid(Idx + 1) : Full;
	}
}

void UInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);

	if (NotesTabButton) NotesTabButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleNotesTab);
	if (HealingTabButton) HealingTabButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleHealingTab);
	if (WeaponsTabButton) WeaponsTabButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleWeaponsTab);
	if (CloseButton) CloseButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleClose);
	if (ActionButton) ActionButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleAction);
}

FReply UInventoryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::I || Key == EKeys::Tab || Key == EKeys::Gamepad_Special_Left)
	{
		OnCloseRequested.ExecuteIfBound();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UInventoryWidget::Init(UInventoryComponent* InInventory)
{
	Inventory = InInventory;
	SetTab(EInventoryTab::Notes);
}

void UInventoryWidget::SetTab(EInventoryTab Tab)
{
	CurrentTab = Tab;
	Selected = 0;
	Rebuild();
}

void UInventoryWidget::HandleNotesTab()   { SetTab(EInventoryTab::Notes); }
void UInventoryWidget::HandleHealingTab() { SetTab(EInventoryTab::Healing); }
void UInventoryWidget::HandleWeaponsTab() { SetTab(EInventoryTab::Weapons); }
void UInventoryWidget::HandleClose()      { OnCloseRequested.ExecuteIfBound(); }

void UInventoryWidget::Rebuild()
{
	if (!ItemList) return;

	BuildEntries();
	Selected = FMath::Min(Selected, Entries.Num() - 1); // -1 when the list is empty

	ItemList->ClearChildren();
	if (RowClass)
	{
		for (int32 i = 0; i < Entries.Num(); ++i)
		{
			UInventoryRowWidget* Row = CreateWidget<UInventoryRowWidget>(this, RowClass);
			if (!Row) continue;

			Row->SetData(i, Entries[i].Label, i == Selected);
			Row->OnRowClicked.BindUObject(this, &UInventoryWidget::HandleRowClicked);
			ItemList->AddChildToVerticalBox(Row);
		}
	}

	UpdateDetails();
	UpdateHeader();
}

void UInventoryWidget::BuildEntries()
{
	Entries.Reset();

	UStoryStateSubsystem* Story = InvUI_GetStory(this);
	if (!Story || !Inventory) return;

	// ---- Notes ----
	if (CurrentTab == EInventoryTab::Notes)
	{
		for (const FGameplayTag& Clue : Story->GetCollectedClues())
		{
			FEntry Entry;
			Entry.Tag = Clue;

			const FText Title = Story->GetClueTitle(Clue);
			Entry.Label = Title.IsEmpty() ? FText::FromString(InvUI_ShortName(Clue)) : Title;
			Entry.Title = Entry.Label;
			Entry.Body = Story->GetClueText(Clue);
			Entries.Add(Entry);
		}
		return;
	}

	// ---- Healing and weapons ----
	UItemDatabase* Database = Inventory->Database;
	if (!Database) return;

	const UPlayerCombatComponent* Combat = Inventory->GetOwner()->FindComponentByClass<UPlayerCombatComponent>();

	for (UItemData* Item : Database->Items)
	{
		if (!Item) continue;

		const int32 Count = Story->GetItemCount(Item->ItemTag);
		if (Count <= 0) continue;

		if (CurrentTab == EInventoryTab::Healing && Item->Type == EItemType::Healing)
		{
			FEntry Entry;
			Entry.Tag = Item->ItemTag;
			Entry.Label = FText::FromString(FString::Printf(TEXT("%s  x%d"), *Item->DisplayName.ToString(), Count));
			Entry.Title = Item->DisplayName;
			Entry.Body = FText::FromString(FString::Printf(TEXT("%s\n\nRestores %.0f health."),
				*Item->Description.ToString(), Item->HealAmount));
			Entry.ActionLabel = FText::FromString(TEXT("Use"));
			Entry.bCanAct = Combat && !Combat->IsDead() && Combat->GetHealth() < Combat->MaxHealth;
			Entries.Add(Entry);
		}
		else if (CurrentTab == EInventoryTab::Weapons && Item->Type == EItemType::Weapon)
		{
			const bool bEquipped = Story->GetEquippedWeapon() == Item->ItemTag;
			const int32 Durability = Story->GetWeaponDurability(Item->ItemTag);

			FString Label = Item->DisplayName.ToString();
			if (Item->MaxDurability > 0)
			{
				Label += FString::Printf(TEXT("  (%d/%d)"), Durability, Item->MaxDurability);
			}
			if (bEquipped)
			{
				Label += TEXT("  [Equipped]");
			}

			FString Body = FString::Printf(TEXT("%s\n\nDamage: %.0f\nRange: %.0f\nSwing time: %.2fs"),
				*Item->Description.ToString(), Item->Damage, Item->Range, Item->WindUpTime + Item->RecoveryTime);
			Body += (Item->MaxDurability > 0)
				? FString::Printf(TEXT("\nDurability: %d / %d"), Durability, Item->MaxDurability)
				: FString(TEXT("\nDurability: unbreakable"));

			FEntry Entry;
			Entry.Tag = Item->ItemTag;
			Entry.Label = FText::FromString(Label);
			Entry.Title = Item->DisplayName;
			Entry.Body = FText::FromString(Body);
			Entry.ActionLabel = FText::FromString(bEquipped ? TEXT("Unequip") : TEXT("Equip"));
			Entry.bCanAct = true;
			Entries.Add(Entry);
		}
	}
}

void UInventoryWidget::UpdateDetails()
{
	if (!DetailTitle || !DetailBody || !ActionButton || !ActionText) return;

	if (!Entries.IsValidIndex(Selected))
	{
		DetailTitle->SetText(FText::FromString(TEXT("Nothing here")));
		DetailBody->SetText(FText::GetEmpty());
		ActionButton->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const FEntry& Entry = Entries[Selected];
	DetailTitle->SetText(Entry.Title);
	DetailBody->SetText(Entry.Body);

	if (Entry.ActionLabel.IsEmpty())
	{
		ActionButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		ActionButton->SetVisibility(ESlateVisibility::Visible);
		ActionButton->SetIsEnabled(Entry.bCanAct);
		ActionText->SetText(Entry.ActionLabel);
	}
}

void UInventoryWidget::UpdateHeader()
{
	if (TabTitleText)
	{
		const TCHAR* Name = (CurrentTab == EInventoryTab::Notes) ? TEXT("Notes")
			: (CurrentTab == EInventoryTab::Healing) ? TEXT("Healing") : TEXT("Weapons");
		TabTitleText->SetText(FText::FromString(Name));
	}

	if (HealthText && Inventory)
	{
		if (const UPlayerCombatComponent* Combat = Inventory->GetOwner()->FindComponentByClass<UPlayerCombatComponent>())
		{
			HealthText->SetText(FText::FromString(
				FString::Printf(TEXT("Health: %.0f / %.0f"), Combat->GetHealth(), Combat->MaxHealth)));
		}
	}
}

void UInventoryWidget::HandleRowClicked(int32 Index)
{
	Selected = Index;

	// Only update highlights. The rows themselves stay, because we're inside one of their click handlers.
	if (ItemList)
	{
		const TArray<UWidget*> Children = ItemList->GetAllChildren();
		for (int32 i = 0; i < Children.Num(); ++i)
		{
			if (UInventoryRowWidget* Row = Cast<UInventoryRowWidget>(Children[i]))
			{
				Row->SetSelected(i == Selected);
			}
		}
	}
	UpdateDetails();
}

void UInventoryWidget::HandleAction()
{
	if (!Inventory || !Entries.IsValidIndex(Selected)) return;

	UItemData* Item = Inventory->FindItem(Entries[Selected].Tag);
	if (!Item) return;

	if (CurrentTab == EInventoryTab::Healing)
	{
		Inventory->UseHealingItem(Item);
	}
	else if (CurrentTab == EInventoryTab::Weapons)
	{
		UStoryStateSubsystem* Story = InvUI_GetStory(this);
		const bool bEquipped = Story && Story->GetEquippedWeapon() == Item->ItemTag;
		Inventory->EquipWeapon(bEquipped ? nullptr : Item);
	}
	Rebuild();
}