#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "RealityAmbienceManager.generated.h"

class UPointLightComponent;
class UPostProcessComponent;
class UStoryStateSubsystem;

USTRUCT(BlueprintType)
struct FRealityLightLook
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	FGameplayTag TargetActorTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	float Intensity = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	bool bFlicker = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	float FlickerMin = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	float FlickerMax = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	float FlickerSpeed = 10.f;
};

USTRUCT(BlueprintType)
struct FRealityMaterialLook
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	FGameplayTag TargetActorTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	int32 Slot = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	UMaterialInterface* Replacement = nullptr;
};

USTRUCT(BlueprintType)
struct FRealityScreenLook
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	float PostProcessWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct FRealityAmbienceLook
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	FName LookName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	TArray<FRealityLightLook> Lights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	TArray<FRealityMaterialLook> Materials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	FRealityScreenLook Screen;
};

UCLASS()
class EPICAIMEGAJAM_API ARealityAmbienceManager : public AActor
{
	GENERATED_BODY()

public:
	ARealityAmbienceManager();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ambience")
	TObjectPtr<UPostProcessComponent> PostProcess;

	// All lights in the level that can be controlled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	TArray<AActor*> TargetLights;

	// All meshes in the level that can have materials swapped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	TArray<AActor*> TargetMeshes;

	// The different ambience looks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	TArray<FRealityAmbienceLook> Looks;

	// Transition speed between looks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ambience")
	float TransitionTime = 2.0f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	virtual void Tick(float DeltaTime) override;

	void RefreshTarget(bool bImmediate = false);
	void TransitionToLook(int32 TargetIndex);
	void ApplyLights();
	void ApplyMaterials();
	void ApplyScreen();

	UFUNCTION()
	void HandleFactChanged(FGameplayTag Fact);

	TWeakObjectPtr<UStoryStateSubsystem> Story;

	// Cached light components from TargetLights
	TArray<TPair<TWeakObjectPtr<AActor>, TWeakObjectPtr<UPointLightComponent>>> CachedLights;

	// Cached mesh components from TargetMeshes
	TArray<TPair<TWeakObjectPtr<AActor>, TWeakObjectPtr<UStaticMeshComponent>>> CachedMeshes;

	// Original materials before swapping
	TArray<TPair<int32, TObjectPtr<UMaterialInterface>>> OriginalMaterials;

	int32 TargetLookIndex = -1;
	int32 CurrentLookIndex = -1;

	float CurWeight = 0.f;
	float SettleTime = 0.f;
	float CurFlicker = 0.f;
};
