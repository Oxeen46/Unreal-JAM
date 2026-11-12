#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "BoardSlotWidget.generated.h"

class UTextBlock;

UCLASS()
class EPICAIMEGAJAM_API UBoardSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitSlot(FGameplayTag InSlotTag, const FText& InLabel, FGameplayTag PlacedClue);

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ClueText;

private:
	FGameplayTag SlotTag;
};