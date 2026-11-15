#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NoteCardWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

UCLASS()
class EPICAIMEGAJAM_API UNoteCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetNote(UTexture2D* Icon, const FText& Name, int32 Order);

protected:
	// All optional. Add the ones you want in the Designer (Is Variable ticked).
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UImage> IconImage;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> NameText;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> OrderText;
};