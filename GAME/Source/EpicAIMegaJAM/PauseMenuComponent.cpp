#include "PauseMenuComponent.h"
#include "PauseMenuWidget.h"
#include "PlayerCombatComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

APlayerController* UPauseMenuComponent::GetPC() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	return Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
}

void UPauseMenuComponent::TogglePauseMenu()
{
	if (Menu) Close();
	else Open();
}

void UPauseMenuComponent::Open()
{
	APlayerController* PC = GetPC();
	if (!PC || !PauseMenuClass) return;

	// Not while another menu (like the storyboard) is open, or while dying
	if (PC->bShowMouseCursor) return;
	if (const UPlayerCombatComponent* Combat = GetOwner()->FindComponentByClass<UPlayerCombatComponent>())
	{
		if (Combat->IsDead()) return;
	}

	Menu = CreateWidget<UPauseMenuWidget>(PC, PauseMenuClass);
	if (!Menu) return;

	Menu->OnResumeRequested.BindUObject(this, &UPauseMenuComponent::Close);
	Menu->AddToViewport(50);

	UGameplayStatics::SetGamePaused(GetWorld(), true);

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(Menu->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(Mode);
	PC->SetShowMouseCursor(true);
}

void UPauseMenuComponent::Close()
{
	if (Menu)
	{
		Menu->RemoveFromParent();
		Menu = nullptr;
	}

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (APlayerController* PC = GetPC())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
}