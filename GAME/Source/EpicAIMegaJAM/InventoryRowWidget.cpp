#include "InventoryRowWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UInventoryRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (RowButton) RowButton->OnClicked.AddDynamic(this, &UInventoryRowWidget::HandleClicked);
}

void UInventoryRowWidget::SetData(int32 InIndex, const FText& Label, bool bSelected)
{
	Index = InIndex;
	if (RowText) RowText->SetText(Label);
	SetSelected(bSelected);
}

void UInventoryRowWidget::SetSelected(bool bSelected)
{
	if (RowButton)
	{
		RowButton->SetBackgroundColor(bSelected ? FLinearColor(1.f, 0.85f, 0.3f, 1.f) : FLinearColor::White);
	}
}

void UInventoryRowWidget::HandleClicked()
{
	OnRowClicked.ExecuteIfBound(Index);
}