#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EPICAIMEGAJAM_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void TryInteract();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	float InteractDistance = 250.f;
};