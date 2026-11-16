#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "StoryReactiveActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UStoryStateSubsystem;

UCLASS()
class EPICAIMEGAJAM_API AStoryReactiveActor : public AActor
{
	GENERATED_BODY()

public:
	AStoryReactiveActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Reactive")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Reactive")
	TObjectPtr<UPointLightComponent> Light;

	// The whole actor only exists while this fact is true (leave empty for no condition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reactive")
	FGameplayTag ShowWhenFact;

	// The whole actor disappears while this fact is true
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reactive")
	FGameplayTag HideWhenFact;

	// Light color per fact. The first fact that is true wins. Leave empty for no light.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reactive")
	TMap<FGameplayTag, FLinearColor> LightColorByFact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reactive")
	FLinearColor DefaultLightColor = FLinearColor(1.f, 0.9f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reactive")
	float LightIntensity = 5000.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleFactChanged(FGameplayTag Fact);

	void Refresh();

	TWeakObjectPtr<UStoryStateSubsystem> Story;
};