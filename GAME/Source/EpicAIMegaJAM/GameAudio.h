#pragma once
#include "CoreMinimal.h"

class USoundBase;
class USoundClass;
class UObject;

// Small helpers for the one-shot UI / pickup / footstep sounds (assets from SAFA-SESLER, loaded by path on first use).
// Everything played here goes through the SFX sound class from Project Settings > Game > Game Options,
// so the main menu's Master / SFX sliders control it.
namespace GameAudio
{
	EPICAIMEGAJAM_API USoundBase* Load(const FString& ObjectPath);

	// The sound classes the Music / SFX sliders control (null if not set in the settings)
	EPICAIMEGAJAM_API USoundClass* GetSfxClass();
	EPICAIMEGAJAM_API USoundClass* GetMusicClass();

	// Plays a loaded sound as a plain 2D sound on the SFX class
	EPICAIMEGAJAM_API void PlaySfx2D(const UObject* Context, USoundBase* Sound, float Volume = 1.f, float Pitch = 1.f);

	// Plays /Game/SAFA-SESLER/<Folder>/<Name> as a plain 2D sound. Folder like "04_Pano_Kagit_ve_Kanitlar_Secilmis"
	EPICAIMEGAJAM_API void Play2D(const UObject* Context, const FString& Folder, const FString& Name, float Volume = 1.f, float PitchMin = 1.f, float PitchMax = 1.f);

	// Picks Name_V01 .. Name_Vnn at random (never the same variant twice in a row)
	EPICAIMEGAJAM_API void PlayVariant2D(const UObject* Context, const FString& Folder, const FString& Name, int32 NumVariants, float Volume = 1.f, float PitchMin = 1.f, float PitchMax = 1.f);

	EPICAIMEGAJAM_API void ClueCollected(const UObject* Context);
	EPICAIMEGAJAM_API void ItemPickedUp(const UObject* Context);
	EPICAIMEGAJAM_API void BoardNotePlaced(const UObject* Context);
	EPICAIMEGAJAM_API void BoardSwitched(const UObject* Context);
}
