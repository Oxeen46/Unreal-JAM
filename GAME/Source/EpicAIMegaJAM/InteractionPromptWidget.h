#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPromptWidget.generated.h"

class UTextBlock;

UCLASS()
class EPICAIMEGAJAM_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPrompt(const FText& Key, const FText& Action);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> KeyText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ActionText;
};