#include "ClueHighlightSubsystem.h"
#include "ClueActor.h"
#include "StoryStateSubsystem.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"

namespace
{
	const TCHAR* OutlineMaterialPath = TEXT("/Game/ModeLM/Materials/M_ClueOutline.M_ClueOutline");
}

bool UClueHighlightSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) return false;
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

TStatId UClueHighlightSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UClueHighlightSubsystem, STATGROUP_Tickables);
}

void UClueHighlightSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, OutlineMaterialPath);
	if (!Material)
	{
		UE_LOG(LogTemp, Warning, TEXT("ClueHighlight: %s not found, clue outlines are off"), OutlineMaterialPath);
		return;
	}
	bMaterialOk = true;

	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;
	Volume = InWorld.SpawnActor<APostProcessVolume>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (Volume)
	{
		Volume->bUnbound = true;
		Volume->BlendWeight = 1.f;
		Volume->Priority = 100.f;
		Volume->Settings.AddBlendable(Material, 1.f);
	}
	Rescan();
	UE_LOG(LogTemp, Log, TEXT("ClueHighlight: tracking %d clue(s) with something to outline"), Entries.Num());
}

void UClueHighlightSubsystem::Deinitialize()
{
	for (FEntry& Entry : Entries) SetLevel(Entry, 0);
	Entries.Reset();
	Known.Reset();
	if (Volume)
	{
		Volume->Destroy();
		Volume = nullptr;
	}
	Super::Deinitialize();
}

void UClueHighlightSubsystem::BuildTargets(FEntry& Entry) const
{
	AClueActor* Clue = Entry.Clue.Get();
	if (!Clue) return;

	Entry.Targets.Reset();
	if (Clue->Mesh && Clue->Mesh->GetStaticMesh())
	{
		Entry.Targets.Add(Clue->Mesh);
	}
	for (AActor* Other : Clue->HighlightActors)
	{
		if (!Other) continue;
		TInlineComponentArray<UStaticMeshComponent*> Comps;
		Other->GetComponents(Comps);
		for (UStaticMeshComponent* C : Comps)
		{
			if (C && C->GetStaticMesh()) Entry.Targets.Add(C);
		}
	}

	if (Entry.Targets.Num() == 0)
	{
		// Mesh-less clue: take the closest small static mesh right next to it
		UPrimitiveComponent* Best = nullptr;
		float BestDist = AutoPickDistance;
		const FVector Where = Clue->GetActorLocation();
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor == Clue || Actor->IsA<AClueActor>()) continue;
			TInlineComponentArray<UStaticMeshComponent*> Comps;
			Actor->GetComponents(Comps);
			for (UStaticMeshComponent* C : Comps)
			{
				if (!C || !C->GetStaticMesh()) continue;
				const FBoxSphereBounds B = C->Bounds;
				if (B.BoxExtent.GetMax() * 2.f > AutoPickMaxSize) continue;
				const float D = FMath::Sqrt(B.GetBox().ComputeSquaredDistanceToPoint(Where));
				if (D < BestDist)
				{
					BestDist = D;
					Best = C;
				}
			}
		}
		if (Best) Entry.Targets.Add(Best);
	}

	FBox Box(ForceInit);
	for (const TWeakObjectPtr<UPrimitiveComponent>& T : Entry.Targets)
	{
		if (T.IsValid()) Box += T->Bounds.GetBox();
	}
	Entry.Center = Box.IsValid ? Box.GetCenter() : Clue->GetActorLocation();
}

void UClueHighlightSubsystem::Rescan()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<AClueActor> It(World); It; ++It)
	{
		AClueActor* Clue = *It;
		if (Known.Contains(Clue)) continue;
		Known.Add(Clue);

		FEntry Entry;
		Entry.Clue = Clue;
		BuildTargets(Entry);
		if (Entry.Targets.Num() > 0)
		{
			Entries.Add(MoveTemp(Entry));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("ClueHighlight: nothing to outline for %s (no mesh and no small mesh nearby)"), *Clue->GetName());
		}
	}
}

void UClueHighlightSubsystem::SetLevel(FEntry& Entry, int32 NewLevel)
{
	if (Entry.Level == NewLevel) return;
	Entry.Level = NewLevel;
	for (const TWeakObjectPtr<UPrimitiveComponent>& T : Entry.Targets)
	{
		if (UPrimitiveComponent* C = T.Get())
		{
			C->SetCustomDepthStencilValue(FMath::Max(NewLevel, 0));
			C->SetRenderCustomDepth(NewLevel > 0);
		}
	}
}

void UClueHighlightSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bMaterialOk) return;

	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC) return;

	RescanTimer -= DeltaTime;
	if (RescanTimer <= 0.f)
	{
		RescanTimer = 2.f;
		Rescan();
	}

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	UStoryStateSubsystem* Story = nullptr;
	if (UGameInstance* GI = World->GetGameInstance())
	{
		Story = GI->GetSubsystem<UStoryStateSubsystem>();
	}

	for (int32 i = Entries.Num() - 1; i >= 0; --i)
	{
		FEntry& Entry = Entries[i];
		AClueActor* Clue = Entry.Clue.Get();
		if (!Clue)
		{
			Entries.RemoveAtSwap(i);
			continue;
		}

		const bool bCollected = Story && Clue->ClueFact.IsValid() && Story->HasClue(Clue->ClueFact);
		const float Dist = FVector::Dist(CamLoc, Entry.Center);
		const bool bWant = !bCollected && !Clue->IsHidden() && Dist < ShowDistance;

		if (bWant)
		{
			Entry.NextTrace -= DeltaTime;
			if (Entry.NextTrace <= 0.f)
			{
				Entry.NextTrace = 0.12f;
				FCollisionQueryParams Params(SCENE_QUERY_STAT(ClueHighlightLOS), false);
				Params.AddIgnoredActor(PC->GetPawn());
				Params.AddIgnoredActor(Clue);
				for (const TWeakObjectPtr<UPrimitiveComponent>& T : Entry.Targets)
				{
					if (T.IsValid()) Params.AddIgnoredActor(T->GetOwner());
				}
				FHitResult Hit;
				Entry.bVisible = !World->LineTraceSingleByChannel(Hit, CamLoc, Entry.Center, ECC_Visibility, Params);
			}
		}
		else
		{
			Entry.bVisible = false;
		}

		int32 Level = 0;
		if (bWant && Entry.bVisible)
		{
			Level = FMath::Clamp(FMath::CeilToInt(3.f * (1.f - Dist / ShowDistance)), 1, 3);
		}
		SetLevel(Entry, Level);
	}
}
