#include "StoryReactiveActor.h"
#include "StoryStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/GameInstance.h"

AStoryReactiveActor::AStoryReactiveActor()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Mesh);
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	Light->SetAttenuationRadius(900.f);
	Light->SetIntensity(0.f);
}

void AStoryReactiveActor::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		Story = GI->GetSubsystem<UStoryStateSubsystem>();
		if (Story.IsValid())
		{
			Story->OnFactChanged.AddDynamic(this, &AStoryReactiveActor::HandleFactChanged);
		}
	}
	Refresh();
}

void AStoryReactiveActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Story.IsValid())
	{
		Story->OnFactChanged.RemoveDynamic(this, &AStoryReactiveActor::HandleFactChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void AStoryReactiveActor::HandleFactChanged(FGameplayTag Fact)
{
	Refresh();
}

void AStoryReactiveActor::Refresh()
{
	bool bVisible = true;
	FLinearColor Color = DefaultLightColor;

	if (Story.IsValid())
	{
		if (ShowWhenFact.IsValid() && !Story->HasFact(ShowWhenFact)) bVisible = false;
		if (HideWhenFact.IsValid() && Story->HasFact(HideWhenFact)) bVisible = false;

		for (const TPair<FGameplayTag, FLinearColor>& Pair : LightColorByFact)
		{
			if (Story->HasFact(Pair.Key))
			{
				Color = Pair.Value;
				break;
			}
		}
	}
	else if (ShowWhenFact.IsValid())
	{
		bVisible = false;
	}

	SetActorHiddenInGame(!bVisible);
	SetActorEnableCollision(bVisible);

	Light->SetLightColor(Color);
	Light->SetIntensity(LightColorByFact.Num() > 0 ? LightIntensity : 0.f);
}