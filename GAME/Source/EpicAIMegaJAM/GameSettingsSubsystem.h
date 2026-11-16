#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameOptionsSave.h"
#include "GameSettingsSubsystem.generated.h"

UCLASS()
class EPICAIMEGAJAM_API UGameSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category="Options")
	FGameOptions GetOptions() const { return Options; }

	// Applies and saves
	UFUNCTION(BlueprintCallable, Category="Options")
	void SetOptions(const FGameOptions& NewOptions);

	UFUNCTION(BlueprintCallable, Category="Options")
	void ResetToDefaults();

	// Call when a level or the main menu starts so audio settings are in effect
	UFUNCTION(BlueprintCallable, Category="Options")
	void ApplyAll();

	UFUNCTION(BlueprintPure, Category="Options")
	float GetMouseSensitivity() const { return Options.MouseSensitivity; }

	UFUNCTION(BlueprintPure, Category="Options")
	bool GetInvertY() const { return Options.bInvertY; }

	UFUNCTION(BlueprintPure, Category="Options")
	float GetRealityEffect() const { return Options.RealityEffect; }

private:
	void ApplyAudio();
	void SaveOptions();
	void LoadOptions();

	FGameOptions Options;
};