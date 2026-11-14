#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "SavePointActor.generated.h"

class UStaticMeshComponent;
class UArrowComponent;
class USaveSlotMenuWidget;
class APlayerController;

UCLASS()
class EPICAIMEGAJAM_API ASavePointActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ASavePointActor();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Save Point")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// Where the player reappears after dying or loading. Move and rotate it in the editor.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Save Point")
	TObjectPtr<UArrowComponent> RespawnPoint;

	// Shown in the save slot list
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save Point")
	FText PlaceName = FText::FromString(TEXT("Unknown place"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save Point")
	FText InteractionText = FText::FromString(TEXT("Save game"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save Point")
	TSubclassOf<USaveSlotMenuWidget> SaveMenuClass;

private:
	void CloseMenu();

	UPROPERTY() TObjectPtr<USaveSlotMenuWidget> Menu;
	UPROPERTY() TObjectPtr<APlayerController> MenuPC;
};