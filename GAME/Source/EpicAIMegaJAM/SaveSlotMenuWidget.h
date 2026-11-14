#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveSlotMenuWidget.generated.h"

class UButton;
class UTextBlock;

UENUM(BlueprintType)
enum class ESlotMenuMode : uint8
{
	Save,
	Load
};

UCLASS()
class EPICAIMEGAJAM_API USaveSlotMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitMenu(ESlotMenuMode InMode);

	// Fired when Back is pressed. The owner removes the widget.
	FSimpleDelegate OnClosed;

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Slot0Button;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Slot1Button;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Slot2Button;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Slot3Button;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Slot0Text;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Slot1Text;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Slot2Text;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> Slot3Text;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> BackButton;

	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> TitleText;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> StatusText;

private:
	UFUNCTION() void HandleSlot0();
	UFUNCTION() void HandleSlot1();
	UFUNCTION() void HandleSlot2();
	UFUNCTION() void HandleSlot3();
	UFUNCTION() void HandleBack();

	void HandleSlot(int32 Index);
	void Refresh();

	UPROPERTY() TArray<TObjectPtr<UButton>> Buttons;
	UPROPERTY() TArray<TObjectPtr<UTextBlock>> Texts;

	ESlotMenuMode Mode = ESlotMenuMode::Load;
};