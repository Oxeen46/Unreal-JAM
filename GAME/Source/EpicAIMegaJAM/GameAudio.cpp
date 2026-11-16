#include "GameAudio.h"
#include "GameOptionsDeveloperSettings.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundClass.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	TMap<FString, USoundBase*> Cache;
	TMap<FString, int32> LastVariant;

	FString ObjectPath(const FString& Folder, const FString& Name)
	{
		return FString::Printf(TEXT("/Game/SAFA-SESLER/%s/%s.%s"), *Folder, *Name, *Name);
	}
}

USoundBase* GameAudio::Load(const FString& Path)
{
	if (USoundBase** Found = Cache.Find(Path))
	{
		return *Found;
	}
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, *Path);
	if (Sound)
	{
		Sound->AddToRoot(); // keep it alive, these are tiny one-shots
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameAudio: sound not found: %s"), *Path);
	}
	Cache.Add(Path, Sound);
	return Sound;
}

USoundClass* GameAudio::GetSfxClass()
{
	const UGameOptionsDeveloperSettings* Config = GetDefault<UGameOptionsDeveloperSettings>();
	return Config ? Config->SfxClass.LoadSynchronous() : nullptr;
}

USoundClass* GameAudio::GetMusicClass()
{
	const UGameOptionsDeveloperSettings* Config = GetDefault<UGameOptionsDeveloperSettings>();
	return Config ? Config->MusicClass.LoadSynchronous() : nullptr;
}

void GameAudio::PlaySfx2D(const UObject* Context, USoundBase* Sound, float Volume, float Pitch)
{
	if (!Context || !Sound) return;
	// an audio component (not PlaySound2D) so the sound can be put on the SFX class and follow the SFX slider
	if (UAudioComponent* Comp = UGameplayStatics::CreateSound2D(Context, Sound, Volume, Pitch, 0.f, nullptr, false, true))
	{
		if (USoundClass* Sfx = GetSfxClass())
		{
			Comp->SoundClassOverride = Sfx;
		}
		Comp->Play();
	}
}

void GameAudio::Play2D(const UObject* Context, const FString& Folder, const FString& Name, float Volume, float PitchMin, float PitchMax)
{
	if (!Context) return;
	if (USoundBase* Sound = Load(ObjectPath(Folder, Name)))
	{
		PlaySfx2D(Context, Sound, Volume, FMath::FRandRange(PitchMin, PitchMax));
	}
}

void GameAudio::PlayVariant2D(const UObject* Context, const FString& Folder, const FString& Name, int32 NumVariants, float Volume, float PitchMin, float PitchMax)
{
	if (NumVariants <= 0) return;
	int32 Pick = FMath::RandRange(1, NumVariants);
	int32& Last = LastVariant.FindOrAdd(Name);
	if (NumVariants > 1 && Pick == Last)
	{
		Pick = (Pick % NumVariants) + 1;
	}
	Last = Pick;
	Play2D(Context, Folder, FString::Printf(TEXT("%s_V%02d"), *Name, Pick), Volume, PitchMin, PitchMax);
}

void GameAudio::ClueCollected(const UObject* Context)
{
	PlayVariant2D(Context, TEXT("04_Pano_Kagit_ve_Kanitlar_Secilmis"), TEXT("Kagit_Cevirme"), 3, 0.8f, 0.97f, 1.03f);
	Play2D(Context, TEXT("04_Pano_Kagit_ve_Kanitlar_Secilmis"), TEXT("Esya_Alindi_V01"), 0.5f);
}

void GameAudio::ItemPickedUp(const UObject* Context)
{
	PlayVariant2D(Context, TEXT("02_Silah_ve_Cep_Secilmis"), TEXT("Cebe_Koyma"), 4, 0.8f, 0.97f, 1.03f);
}

void GameAudio::BoardNotePlaced(const UObject* Context)
{
	PlayVariant2D(Context, TEXT("04_Pano_Kagit_ve_Kanitlar_Secilmis"), TEXT("Pano_Kart_Koy"), 2, 0.9f);
}

void GameAudio::BoardSwitched(const UObject* Context)
{
	PlayVariant2D(Context, TEXT("04_Pano_Kagit_ve_Kanitlar_Secilmis"), TEXT("Pano_Gecis"), 2, 0.8f);
}
