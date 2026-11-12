#pragma once
#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "GameplayTagContainer.h"
#include "ClueDragDropOperation.generated.h"

UCLASS()
class EPICAIMEGAJAM_API UClueDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameplayTag ClueTag;
};