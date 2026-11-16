#include "InventoryComponent.h"
#include "InventoryWidget.h"
#include "ItemData.h"
#include "PlayerCombatComponent.h"
#include "StoryStateSubsystem.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const UPlayerCombatComponent* Combat = GetOwner()->FindComponentByClass<UPlayerCombatComponent>())
	{
		FistDamage = Combat->AttackDamage;
		FistRange = Combat->AttackRange;
		FistWindUp = Combat->WindUpTime;
		FistRecovery = Combat->RecoveryTime;
		bFistsCaptured = true;
	}

	// Restores the equipped weapon after a level reload or load
	ApplyEquipped();
}

UStoryStateSubsystem* UInventoryComponent::GetStory() const
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr;
}

APlayerController* UInventoryComponent::GetPC() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	return Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
}

UItemData* UInventoryComponent::FindItem(FGameplayTag Tag) const
{
	return Database ? Database->FindItem(Tag) : nullptr;
}

int32 UInventoryComponent::AddItem(UItemData* Item, int32 Amount)
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Item || !Story || Amount <= 0) return 0;

	const int32 Current = Story->GetItemCount(Item->ItemTag);
	const int32 Space = FMath::Max(0, Item->MaxStack - Current);
	const int32 Added = FMath::Min(Amount, Space);
	if (Added <= 0) return 0;

	Story->AddItem(Item->ItemTag, Added);

	// A fresh weapon starts at full durability
	if (Item->Type == EItemType::Weapon && Current == 0 && Item->MaxDurability > 0)
	{
		Story->SetWeaponDurability(Item->ItemTag, Item->MaxDurability);
	}
	return Added;
}

bool UInventoryComponent::UseHealingItem(UItemData* Item)
{
	UStoryStateSubsystem* Story = GetStory();
	UPlayerCombatComponent* Combat = GetOwner()->FindComponentByClass<UPlayerCombatComponent>();
	if (!Item || !Story || !Combat || Item->Type != EItemType::Healing) return false;

	if (Combat->IsDead() || Combat->GetHealth() >= Combat->MaxHealth) return false;
	if (!Story->RemoveItem(Item->ItemTag, 1)) return false;

	Combat->Heal(Item->HealAmount);
	return true;
}

void UInventoryComponent::EquipWeapon(UItemData* Weapon)
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Story) return;

	Story->SetEquippedWeapon(Weapon ? Weapon->ItemTag : FGameplayTag());
	ApplyEquipped();
}

void UInventoryComponent::ApplyEquipped()
{
	UStoryStateSubsystem* Story = GetStory();
	UPlayerCombatComponent* Combat = GetOwner()->FindComponentByClass<UPlayerCombatComponent>();
	if (!Story || !Combat || !bFistsCaptured) return;

	UItemData* Weapon = nullptr;
	const FGameplayTag Equipped = Story->GetEquippedWeapon();
	if (Equipped.IsValid())
	{
		Weapon = FindItem(Equipped);
		if (!Weapon || Story->GetItemCount(Equipped) <= 0)
		{
			Weapon = nullptr;
			Story->SetEquippedWeapon(FGameplayTag());
		}
	}

	if (Weapon)
	{
		Combat->AttackDamage = Weapon->Damage;
		Combat->AttackRange = Weapon->Range;
		Combat->WindUpTime = Weapon->WindUpTime;
		Combat->RecoveryTime = Weapon->RecoveryTime;
	}
	else
	{
		Combat->AttackDamage = FistDamage;
		Combat->AttackRange = FistRange;
		Combat->WindUpTime = FistWindUp;
		Combat->RecoveryTime = FistRecovery;
	}
}

void UInventoryComponent::OnWeaponHit()
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Story) return;

	const FGameplayTag Equipped = Story->GetEquippedWeapon();
	UItemData* Weapon = Equipped.IsValid() ? FindItem(Equipped) : nullptr;
	if (!Weapon || Weapon->MaxDurability <= 0) return;

	const int32 Durability = Story->GetWeaponDurability(Equipped) - 1;
	Story->SetWeaponDurability(Equipped, Durability);
	if (Durability > 0) return;

	// It broke
	Story->RemoveItem(Equipped, 1);
	if (Story->GetItemCount(Equipped) > 0)
	{
		Story->SetWeaponDurability(Equipped, Weapon->MaxDurability); // next one of the same kind
	}
	else
	{
		Story->SetEquippedWeapon(FGameplayTag());
	}
	ApplyEquipped();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange,
			FString::Printf(TEXT("%s broke."), *Weapon->DisplayName.ToString()));
	}
}

void UInventoryComponent::ToggleInventory()
{
	if (Widget) Close();
	else Open();
}

void UInventoryComponent::Open()
{
	APlayerController* PC = GetPC();
	if (!PC || !InventoryWidgetClass || Widget) return;

	// Not while another menu is open, or while dying
	if (PC->bShowMouseCursor) return;
	if (const UPlayerCombatComponent* Combat = GetOwner()->FindComponentByClass<UPlayerCombatComponent>())
	{
		if (Combat->IsDead()) return;
	}

	Widget = CreateWidget<UInventoryWidget>(PC, InventoryWidgetClass);
	if (!Widget) return;

	Widget->OnCloseRequested.BindUObject(this, &UInventoryComponent::Close);
	Widget->Init(this);
	Widget->AddToViewport(40);

	UGameplayStatics::SetGamePaused(GetWorld(), true);

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(Widget->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(Mode);
	PC->SetShowMouseCursor(true);
}

void UInventoryComponent::Close()
{
	if (Widget)
	{
		Widget->RemoveFromParent();
		Widget = nullptr;
	}

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (APlayerController* PC = GetPC())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
}