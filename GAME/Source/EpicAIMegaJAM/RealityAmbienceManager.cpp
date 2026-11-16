#include "RealityAmbienceManager.h"
#include "StoryStateSubsystem.h"
#include "Components/PointLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"

ARealityAmbienceManager::ARealityAmbienceManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(GetRootComponent());
	PostProcess->bUnbound = true;
	PostProcess->BlendWeight = 0.f;
}

void ARealityAmbienceManager::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		Story = GI->GetSubsystem<UStoryStateSubsystem>();
		if (Story.IsValid())
		{
			Story->OnFactChanged.AddDynamic(this, &ARealityAmbienceManager::HandleFactChanged);
		}
	}

	// Cache light components from target actors
	for (AActor* Actor : TargetLights)
	{
		if (Actor)
		{
			for (UActorComponent* Comp : Actor->GetComponents())
			{
				if (UPointLightComponent* Light = Cast<UPointLightComponent>(Comp))
				{
					CachedLights.Emplace(TWeakObjectPtr<AActor>(Actor), TWeakObjectPtr<UPointLightComponent>(Light));
					break;
				}
			}
		}
	}

	// Cache mesh components from target actors
	for (AActor* Actor : TargetMeshes)
	{
		if (Actor)
		{
			for (UActorComponent* Comp : Actor->GetComponents())
			{
				if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Comp))
				{
					CachedMeshes.Emplace(TWeakObjectPtr<AActor>(Actor), TWeakObjectPtr<UStaticMeshComponent>(Mesh));

					// Store original materials
					for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
					{
						OriginalMaterials.Emplace(i, Mesh->GetMaterial(i));
					}
					break;
				}
			}
		}
	}

	RefreshTarget(true);
}

void ARealityAmbienceManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Story.IsValid())
	{
		Story->OnFactChanged.RemoveDynamic(this, &ARealityAmbienceManager::HandleFactChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void ARealityAmbienceManager::HandleFactChanged(FGameplayTag Fact)
{
	(void)Fact;
	RefreshTarget(false);
}

void ARealityAmbienceManager::RefreshTarget(bool bImmediate)
{
	if (!Story.IsValid() || Looks.Num() == 0)
	{
		return;
	}

	// Determine target look based on active storyline
	// For now, use the first look as default
	int32 NewTarget = 0;

	if (CurrentLookIndex != NewTarget)
	{
		if (bImmediate)
		{
			CurrentLookIndex = NewTarget;
			CurWeight = 1.f;
			SettleTime = 0.f;
		}
		else
		{
			TransitionToLook(NewTarget);
		}
	}
}

void ARealityAmbienceManager::TransitionToLook(int32 TargetIndex)
{
	if (!Looks.IsValidIndex(TargetIndex))
	{
		return;
	}

	TargetLookIndex = TargetIndex;
	CurrentLookIndex = TargetIndex;
	CurWeight = 0.f;
	SettleTime = TransitionTime + 0.5f;
}

void ARealityAmbienceManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SettleTime > 0.f)
	{
		SettleTime -= DeltaTime;
		CurWeight = FMath::Clamp(CurWeight + (DeltaTime / FMath::Max(TransitionTime, 0.01f)), 0.f, 1.f);
	}

	// Apply changes during transition or while settling
	if (SettleTime > 0.f || CurFlicker > 0.001f)
	{
		ApplyLights();
		ApplyMaterials();
		ApplyScreen();
	}
}

void ARealityAmbienceManager::ApplyLights()
{
	if (!Looks.IsValidIndex(CurrentLookIndex))
	{
		return;
	}

	const FRealityAmbienceLook& Look = Looks[CurrentLookIndex];

	for (const TPair<TWeakObjectPtr<AActor>, TWeakObjectPtr<UPointLightComponent>>& Pair : CachedLights)
	{
		if (UPointLightComponent* Light = Pair.Value.Get())
		{
			// Find matching light look by actor tag
			const FRealityLightLook* MatchLight = nullptr;
			for (const FRealityLightLook& LightLook : Look.Lights)
			{
    if (Pair.Key.IsValid() && LightLook.TargetActorTag.GetTagName() == Pair.Key->GetFName())
				{
					MatchLight = &LightLook;
					break;
				}
			}

			if (MatchLight)
			{
				Light->SetLightColor(MatchLight->Color);
				Light->SetIntensity(MatchLight->Intensity * CurWeight);

				if (MatchLight->bFlicker && SettleTime <= 0.f)
				{
					CurFlicker = FMath::RandRange(MatchLight->FlickerMin, MatchLight->FlickerMax);
					Light->SetIntensity(Light->Intensity * CurFlicker);
				}
			}
		}
	}
}

void ARealityAmbienceManager::ApplyMaterials()
{
	if (!Looks.IsValidIndex(CurrentLookIndex))
	{
		return;
	}

	const FRealityAmbienceLook& Look = Looks[CurrentLookIndex];

	for (const TPair<TWeakObjectPtr<AActor>, TWeakObjectPtr<UStaticMeshComponent>>& Pair : CachedMeshes)
	{
		if (UStaticMeshComponent* Mesh = Pair.Value.Get())
		{
			for (const FRealityMaterialLook& Entry : Look.Materials)
			{
    if (Pair.Key.IsValid() && Entry.TargetActorTag.GetTagName() == Pair.Key->GetFName())
				{
					UMaterialInterface* Replacement = Entry.Replacement;
					UMaterialInterface* FinalMaterial = Replacement;

					// If no replacement, use the original material
					if (!Replacement)
					{
						for (const TPair<int32, TObjectPtr<UMaterialInterface>>& Orig : OriginalMaterials)
						{
							if (Orig.Key == Entry.Slot)
							{
								FinalMaterial = Orig.Value.Get();
								break;
							}
						}
					}

					if (FinalMaterial)
					{
						Mesh->SetMaterial(Entry.Slot, FinalMaterial);
					}
				}
			}
		}
	}
}

void ARealityAmbienceManager::ApplyScreen()
{
	if (!PostProcess)
	{
		return;
	}

	if (!Looks.IsValidIndex(CurrentLookIndex))
	{
		return;
	}

	const FRealityAmbienceLook& Look = Looks[CurrentLookIndex];
	PostProcess->BlendWeight = CurWeight * Look.Screen.PostProcessWeight;
}