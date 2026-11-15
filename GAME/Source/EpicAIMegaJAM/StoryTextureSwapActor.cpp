#include "StoryTextureSwapActor.h"
#include "StoryStateSubsystem.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

AStoryTextureSwapActor::AStoryTextureSwapActor()
{
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	PrimaryActorTick.bCanEverTick = false;
}

void AStoryTextureSwapActor::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		Story = GI->GetSubsystem<UStoryStateSubsystem>();
		if (Story.IsValid())
		{
			Story->OnFactChanged.AddDynamic(this, &AStoryTextureSwapActor::HandleFactChanged);
		}
	}

	Scan();
	Refresh();

	if (RescanInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(RescanTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			// pick up meshes that streamed in later and keep them in the current state
			Scan();
			if (bDecayed) Apply();
		}), RescanInterval, true);
	}
}

void AStoryTextureSwapActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RescanTimer);
	if (Story.IsValid())
	{
		Story->OnFactChanged.RemoveDynamic(this, &AStoryTextureSwapActor::HandleFactChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void AStoryTextureSwapActor::HandleFactChanged(FGameplayTag Fact)
{
	Refresh();
}

bool AStoryTextureSwapActor::IsDecayed() const
{
	if (!Story.IsValid()) return false;
	for (const FGameplayTag& Tag : DecayFacts)
	{
		if (Tag.IsValid() && Story->HasFact(Tag)) return true;
	}
	return false;
}

void AStoryTextureSwapActor::Scan()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		TInlineComponentArray<UStaticMeshComponent*> Comps;
		It->GetComponents(Comps);
		for (UStaticMeshComponent* Comp : Comps)
		{
			if (!Comp || Known.Contains(Comp)) continue;
			UStaticMesh* Mesh = Comp->GetStaticMesh();
			if (!Mesh) continue;
			Known.Add(Comp);

			FCompSwap Entry;
			Entry.Comp = Comp;
			const int32 Num = Comp->GetNumMaterials();
			for (int32 i = 0; i < Num; ++i)
			{
				UMaterialInterface* Clean = Comp->GetMaterial(i);
				UMaterialInterface* Decayed = Mesh->GetMaterial(i);
				if (Clean)
				{
					if (const TObjectPtr<UMaterialInterface>* Explicit = ReplaceMaterials.Find(Clean))
					{
						if (*Explicit) Decayed = *Explicit;
					}
				}
				// Never swap to an engine default (e.g. the grey grid of the basic Cube): that is not an "old" look
				const bool bRealDecayed = Decayed && !Decayed->GetPathName().StartsWith(TEXT("/Engine/"));
				if (Clean && bRealDecayed && Clean != Decayed)
				{
					FSlotSwap S;
					S.Slot = i;
					S.Clean = Clean;
					S.Decayed = Decayed;
					Entry.Slots.Add(S);
				}
			}
			if (Entry.Slots.Num() > 0)
			{
				Entries.Add(MoveTemp(Entry));
			}
		}
	}
}

void AStoryTextureSwapActor::Apply()
{
	for (const FCompSwap& Entry : Entries)
	{
		UStaticMeshComponent* Comp = Entry.Comp.Get();
		if (!Comp) continue;
		for (const FSlotSwap& S : Entry.Slots)
		{
			UMaterialInterface* Target = bDecayed ? S.Decayed.Get() : S.Clean.Get();
			if (Target) Comp->SetMaterial(S.Slot, Target);
		}
	}
}

void AStoryTextureSwapActor::Refresh()
{
	const bool bNew = IsDecayed();
	if (bNew == bDecayed && Entries.Num() > 0) return;
	bDecayed = bNew;
	Apply();
}
