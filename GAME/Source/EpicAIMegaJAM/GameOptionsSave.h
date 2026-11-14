#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameOptionsSave.generated.h"

USTRUCT(BlueprintType)
struct FGameOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) float MasterVolume = 1.f;
	UPROPERTY(BlueprintReadWrite) float MusicVolume = 0.8f;
	UPROPERTY(BlueprintReadWrite) float SfxVolume = 1.f;
	UPROPERTY(BlueprintReadWrite) float MouseSensitivity = 1.f;
	UPROPERTY(BlueprintReadWrite) bool bInvertY = false;

	// 0 = no dimming effect on reality changes, 1 = full
	UPROPERTY(BlueprintReadWrite) float RealityEffect = 1.f;
};

UCLASS()
class EPICAIMEGAJAM_API UGameOptionsSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() FGameOptions Options;
};