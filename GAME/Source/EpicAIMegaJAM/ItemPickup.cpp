#include "ItemPickup.h"
#include "ItemData.h"
#include "InventoryComponent.h"
#include "StoryStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AItemPickup::AItemPickup()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	if (CubeFinder.Succeeded()) Mesh->SetStaticMesh(CubeFinder.Object);
	Mesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

FString AItemPickup::PickupId() const
{
	return FString::Printf(TEXT("%s/%s"), *UGameplayStatics::GetCurrentLevelName(this, true), *GetName());
}

void AItemPickup::BeginPlay()
{
	Super::BeginPlay();

	// Already picked up before the checkpoint we reloaded from
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UStoryStateSubsystem* Story = GI->GetSubsystem<UStoryStateSubsystem>())
		{
			if (Story->IsPickupCollected(PickupId()))
			{
				Destroy();
			}
		}
	}
}

FText AItemPickup::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	if (!Item) return FText::GetEmpty();
	return FText::FromString(FString::Printf(TEXT("Pick up %s"), *Item->DisplayName.ToString()));
}

void AItemPickup::Interact_Implementation(AActor* Interactor)
{
	UInventoryComponent* Inventory = Interactor ? Interactor->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!Inventory || !Item) return;

	const int32 Added = Inventory->AddItem(Item, Count);
	if (Added <= 0)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
				FString::Printf(TEXT("You can't carry any more %s."), *Item->DisplayName.ToString()));
		}
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
			FString::Printf(TEXT("Picked up %s x%d"), *Item->DisplayName.ToString(), Added));
	}

	Count -= Added;
	if (Count <= 0)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UStoryStateSubsystem* Story = GI->GetSubsystem<UStoryStateSubsystem>())
			{
				Story->MarkPickupCollected(PickupId());
			}
		}
		Destroy();
	}
}