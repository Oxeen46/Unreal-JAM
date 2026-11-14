#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "ItemPickup.generated.h"

class UStaticMeshComponent;
class UItemData;

UCLASS()
class EPICAIMEGAJAM_API AItemPickup : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AItemPickup();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
	TObjectPtr<UItemData> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
	int32 Count = 1;

protected:
	virtual void BeginPlay() override;

private:
	FString PickupId() const;
};