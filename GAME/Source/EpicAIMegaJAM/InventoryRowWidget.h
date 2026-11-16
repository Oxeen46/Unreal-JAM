#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryRowWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UTexture2D;

DECLARE_DELEGATE_OneParam(FOnInventoryRowClicked, int32);

UCLASS()
class EPICAIMEGAJAM_API UInventoryRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetData(int32 InIndex, const FText& Label, bool bSelected, UTexture2D* Icon = nullptr);
	void SetSelected(bool bSelected);

	FOnInventoryRowClicked OnRowClicked;

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> RowButton;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> RowText;
	UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UImage> RowIcon;

private:
	UFUNCTION()
	void HandleClicked();

	int32 Index = INDEX_NONE;
};