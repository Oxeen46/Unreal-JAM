#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KeyHintWidget.generated.h"

class UTextBlock;

// A small key-cap icon with a label in the bottom-left corner of the screen, e.g. [ <- Backspace ]  Leave.
// Built in C++ (no Blueprint). Used by the storyboard to show how to leave it.
UCLASS()
class EPICAIMEGAJAM_API UKeyHintWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetTexts(const FText& Key, const FText& Action);

protected:
	virtual bool Initialize() override;

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> KeyText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ActionText;
};
