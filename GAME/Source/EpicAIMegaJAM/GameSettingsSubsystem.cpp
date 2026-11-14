#include "GameSettingsSubsystem.h"
#include "GameOptionsDeveloperSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

namespace
{
	const FString OptionsSlot = TEXT("GameOptions");
}

void UGameSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOptions();
}

void UGameSettingsSubsystem::LoadOptions()
{
	Options = FGameOptions();

	if (UGameplayStatics::DoesSaveGameExist(OptionsSlot, 0))
	{
		if (const UGameOptionsSave* Save = Cast<UGameOptionsSave>(UGameplayStatics::LoadGameFromSlot(OptionsSlot, 0)))
		{
			Options = Save->Options;
		}
	}
}

void UGameSettingsSubsystem::SaveOptions()
{
	if (UGameOptionsSave* Save = Cast<UGameOptionsSave>(UGameplayStatics::CreateSaveGameObject(UGameOptionsSave::StaticClass())))
	{
		Save->Options = Options;
		UGameplayStatics::SaveGameToSlot(Save, OptionsSlot, 0);
	}
}

void UGameSettingsSubsystem::SetOptions(const FGameOptions& NewOptions)
{
	Options = NewOptions;
	SaveOptions();
	ApplyAudio();
}

void UGameSettingsSubsystem::ResetToDefaults()
{
	SetOptions(FGameOptions());
}

void UGameSettingsSubsystem::ApplyAll()
{
	ApplyAudio();
}

void UGameSettingsSubsystem::ApplyAudio()
{
	UWorld* World = GetWorld();

	// Master volume works with no assets at all
	FAudioDevice* Device = World ? World->GetAudioDeviceRaw() : nullptr;
	if (!Device && GEngine)
	{
		Device = GEngine->GetMainAudioDeviceRaw();
	}
	if (Device)
	{
		Device->SetTransientPrimaryVolume(Options.MasterVolume);
	}

	// Music and SFX need the assets from Project Settings > Game > Game Options
	const UGameOptionsDeveloperSettings* Config = GetDefault<UGameOptionsDeveloperSettings>();
	USoundMix* Mix = Config ? Config->VolumeMix.LoadSynchronous() : nullptr;
	if (!World || !Mix) return;

	UGameplayStatics::SetBaseSoundMix(World, Mix);

	if (USoundClass* Music = Config->MusicClass.LoadSynchronous())
	{
		UGameplayStatics::SetSoundMixClassOverride(World, Mix, Music, Options.MusicVolume, 1.f, 0.f, true);
	}
	if (USoundClass* Sfx = Config->SfxClass.LoadSynchronous())
	{
		UGameplayStatics::SetSoundMixClassOverride(World, Mix, Sfx, Options.SfxVolume, 1.f, 0.f, true);
	}
}