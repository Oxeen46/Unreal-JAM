#include "SaveSlotMenuWidget.h"
#include "StoryStateSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include  "Engine/Engine.h"

namespace
{
	UStoryStateSubsystem* SlotMenu_GetStory(const UObject* Ctx)
	{
		UWorld* World = Ctx ? Ctx->GetWorld() : nullptr;
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr;
	}
}

void USaveSlotMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Buttons = { Slot0Button, Slot1Button, Slot2Button, Slot3Button };
	Texts = { Slot0Text, Slot1Text, Slot2Text, Slot3Text };

	if (Slot0Button) Slot0Button->OnClicked.AddDynamic(this, &USaveSlotMenuWidget::HandleSlot0);
	if (Slot1Button) Slot1Button->OnClicked.AddDynamic(this, &USaveSlotMenuWidget::HandleSlot1);
	if (Slot2Button) Slot2Button->OnClicked.AddDynamic(this, &USaveSlotMenuWidget::HandleSlot2);
	if (Slot3Button) Slot3Button->OnClicked.AddDynamic(this, &USaveSlotMenuWidget::HandleSlot3);
	if (BackButton) BackButton->OnClicked.AddDynamic(this, &USaveSlotMenuWidget::HandleBack);

	if (GEngine && (!Slot0Button || !Slot1Button || !Slot2Button || !Slot3Button || !BackButton ||
		!Slot0Text || !Slot1Text || !Slot2Text || !Slot3Text))
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
			TEXT("WBP_SaveSlotMenu is missing Slot0-3 Buttons/Texts or BackButton (names must match, Is Variable ticked)."));
	}
}

void USaveSlotMenuWidget::InitMenu(ESlotMenuMode InMode)
{
	Mode = InMode;
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(Mode == ESlotMenuMode::Save ? TEXT("Save Game") : TEXT("Load Game")));
	}
	if (StatusText) StatusText->SetText(FText::GetEmpty());
	Refresh();
}

void USaveSlotMenuWidget::Refresh()
{
	UStoryStateSubsystem* Story = SlotMenu_GetStory(this);
	if (!Story) return;

	for (int32 i = 0; i < Buttons.Num(); ++i)
	{
		if (!Buttons[i] || !Texts[i]) continue;

		const FSaveSlotInfo Info = Story->GetSlotInfo(i);

		FString Text = (i == 0) ? TEXT("Autosave") : FString::Printf(TEXT("Slot %d"), i);
		Text += Info.bExists
			? FString::Printf(TEXT("\n%s  -  %s"), *Info.Place, *Info.TimeText)
			: FString(TEXT("\n(empty)"));
		Texts[i]->SetText(FText::FromString(Text));

		const bool bEnabled = (Mode == ESlotMenuMode::Save) ? (i != 0) : Info.bExists;
		Buttons[i]->SetIsEnabled(bEnabled);
	}
}
void USaveSlotMenuWidget::HandleSlot(int32 Index)
{
	UStoryStateSubsystem* Story = SlotMenu_GetStory(this);
	if (!Story) return;

	if (Mode == ESlotMenuMode::Save)
	{
		const bool bOk = Story->SaveToSlot(Index, FString::Printf(TEXT("Slot %d"), Index));
		if (StatusText) StatusText->SetText(FText::FromString(bOk ? TEXT("Saved.") : TEXT("Save failed.")));
		Refresh();
		return;
	}

	// Load: unpause first so the new level starts running
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	if (!Story->ContinueFromSlot(Index))
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);
		if (StatusText) StatusText->SetText(FText::FromString(TEXT("Couldn't load that save.")));
	}
}

void USaveSlotMenuWidget::HandleSlot0() { HandleSlot(0); }
void USaveSlotMenuWidget::HandleSlot1() { HandleSlot(1); }
void USaveSlotMenuWidget::HandleSlot2() { HandleSlot(2); }
void USaveSlotMenuWidget::HandleSlot3() { HandleSlot(3); }

void USaveSlotMenuWidget::HandleBack()
{
	OnClosed.ExecuteIfBound();
}