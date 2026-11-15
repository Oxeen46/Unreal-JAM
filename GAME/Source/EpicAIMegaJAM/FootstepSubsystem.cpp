#include "FootstepSubsystem.h"
#include "GameAudio.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"

namespace
{
	const TCHAR* StepFolder = TEXT("01_Oyuncu_ve_Beden_Secilmis");
}

bool UFootstepSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) return false;
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

TStatId UFootstepSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFootstepSubsystem, STATGROUP_Tickables);
}

EFootSurface UFootstepSubsystem::SurfaceFor(const UPrimitiveComponent* Floor) const
{
	FString Name;
	if (const UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(Floor))
	{
		int32 Slot = 0;
		if (const UStaticMesh* Mesh = SM->GetStaticMesh())
		{
			const TArray<FStaticMaterial>& Mats = Mesh->GetStaticMaterials();
			for (int32 i = 0; i < Mats.Num(); ++i)
			{
				if (Mats[i].MaterialSlotName.ToString().Contains(TEXT("Floor")))
				{
					Slot = i;
					break;
				}
			}
		}
		if (const UMaterialInterface* Mat = SM->GetMaterial(Slot))
		{
			Name = Mat->GetName();
		}
	}
	Name = Name.ToLower();

	auto Has = [&Name](std::initializer_list<const TCHAR*> Words)
	{
		for (const TCHAR* W : Words)
		{
			if (Name.Contains(W)) return true;
		}
		return false;
	};

	if (Has({TEXT("carpet"), TEXT("rug"), TEXT("fabric"), TEXT("velvet")})) return EFootSurface::Carpet;
	if (Has({TEXT("wood"), TEXT("teak"), TEXT("oak"), TEXT("parquet"), TEXT("plank"), TEXT("timber")})) return EFootSurface::Wood;
	if (Has({TEXT("marble"), TEXT("tile"), TEXT("concrete"), TEXT("linoleum"), TEXT("stone"), TEXT("brick"), TEXT("plaster"), TEXT("floor")})) return EFootSurface::Stone;
	return EFootSurface::Wood;
}

void UFootstepSubsystem::PlayStep(EFootSurface Surface)
{
	int32 Count = 1;
	FString Base;
	switch (Surface)
	{
	case EFootSurface::Carpet: Base = TEXT("Yurume_Hali"); Count = 2; break;
	case EFootSurface::Stone: Base = TEXT("Yurume_Tas"); Count = 1; break;
	default: Base = TEXT("Yurume_Ahsap"); Count = 5; break;
	}

	if (Surface == EFootSurface::Stone)
	{
		// two stone variants: Yurume_Tas_V01 and Yurume_Tas_Alt_V01
		const bool bAlt = (++LastIndex % 2) == 1;
		GameAudio::Play2D(this, StepFolder, bAlt ? TEXT("Yurume_Tas_Alt_V01") : TEXT("Yurume_Tas_V01"), Volume, 0.94f, 1.06f);
	}
	else
	{
		GameAudio::PlayVariant2D(this, StepFolder, Base, Count, Volume, 0.94f, 1.06f);
	}
}

void UFootstepSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	ACharacter* Character = PC ? Cast<ACharacter>(PC->GetPawn()) : nullptr;
	UCharacterMovementComponent* Move = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Move || PC->IsPaused()) return;

	const float Speed = Character->GetVelocity().Size2D();
	if (!Move->IsMovingOnGround() || Speed < 25.f)
	{
		Travelled = StrideLength * 0.6f; // the first step comes quickly after starting to walk
		return;
	}

	Travelled += Speed * DeltaTime;
	if (Travelled < StrideLength) return;
	Travelled = 0.f;

	const float Reach = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 60.f;
	const FVector Start = Character->GetActorLocation();
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FootstepTrace), false, Character);
	const EFootSurface Surface = World->LineTraceSingleByChannel(Hit, Start, Start - FVector(0.f, 0.f, Reach), ECC_Visibility, Params)
		? SurfaceFor(Hit.GetComponent())
		: EFootSurface::Wood;
	PlayStep(Surface);
}
