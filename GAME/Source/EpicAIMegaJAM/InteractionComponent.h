#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UInputAction;
class UInteractionPromptWidget;
class APlayerController;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EPICAIMEGAJAM_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	UFUNCTION(BlueprintCallable, Category="Interaction")
	void TryInteract();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	float InteractDistance = 250.f;

	// Used to look up which key to show, e.g. IA_Interact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	TSubclassOf<UInteractionPromptWidget> PromptWidgetClass;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	AActor* FindTarget() const;
	APlayerController* GetPC() const;
	FText GetKeyText() const;

	UPROPERTY()
	TObjectPtr<UInteractionPromptWidget> PromptWidget;
};