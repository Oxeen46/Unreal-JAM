#include "EndScreenTrigger.h"
#include "EndScreenWidget.h"
#include "StoryStateSubsystem.h"
#include "Components/BoxComponent.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

AEndScreenTrigger::AEndScreenTrigger()
{
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);
	Trigger->SetBoxExtent(FVector(150.f, 150.f, 120.f));
	Trigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Trigger->SetGenerateOverlapEvents(true);
	Trigger->SetHiddenInGame(true);
	PrimaryActorTick.bCanEverTick = false;
}

void AEndScreenTrigger::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AEndScreenTrigger::HandleBeginOverlap);
}

void AEndScreenTrigger::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bShown) return;

	const APawn* Pawn = Cast<APawn>(OtherActor);
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || !PC->IsLocalController()) return;

	bShown = true;
	Widget = CreateWidget<UEndScreenWidget>(PC, UEndScreenWidget::StaticClass());
	if (Widget)
	{
		Widget->AddToViewport(100);
		Widget->Show(Message, FadeTime);
	}
	if (bStopMovement)
	{
		PC->SetIgnoreMoveInput(true);
	}
	if (bReturnToMenu)
	{
		GetWorldTimerManager().SetTimer(MenuTimer, this, &AEndScreenTrigger::GoToMenu, FMath::Max(FadeTime, 0.f) + FMath::Max(HoldTime, 0.f), false);
	}
}

void AEndScreenTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(MenuTimer);
	Super::EndPlay(EndPlayReason);
}

void AEndScreenTrigger::GoToMenu()
{
	// a new run starts clean (saved games on disk are not touched)
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UStoryStateSubsystem* Story = GI->GetSubsystem<UStoryStateSubsystem>())
		{
			Story->ResetAllState();
		}
	}
	UGameplayStatics::OpenLevel(this, MenuLevel);
}
