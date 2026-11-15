#include "LevelLoadTrigger.h"
#include "ChapterTitleWidget.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ALevelLoadTrigger::ALevelLoadTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    SetRootComponent(TriggerBox);

    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerBox->SetGenerateOverlapEvents(true);
}

void ALevelLoadTrigger::BeginPlay()
{
    Super::BeginPlay();

    TriggerBox->OnComponentBeginOverlap.AddDynamic(
        this,
        &ALevelLoadTrigger::OnPlayerEnter
    );
}

void ALevelLoadTrigger::OnPlayerEnter(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (bTriggered)
        return;

    APawn* PlayerPawn = Cast<APawn>(OtherActor);

    if (!PlayerPawn || !PlayerPawn->IsPlayerControlled())
        return;

    bTriggered = true;

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

    if (PC && ChapterWidgetClass)
    {
        UChapterTitleWidget* ChapterWidget =
            CreateWidget<UChapterTitleWidget>(PC, ChapterWidgetClass);

        if (ChapterWidget)
        {
            ChapterWidget->SetChapterTitle(ChapterTitle);
            ChapterWidget->AddToViewport(100);
            ChapterWidget->PlayChapterAnimation();
        }
    }

    GetWorldTimerManager().SetTimer(
        LoadLevelTimer,
        this,
        &ALevelLoadTrigger::LoadNextLevel,
        DisplayDuration,
        false
    );
}

void ALevelLoadTrigger::LoadNextLevel()
{
    if (!LevelToLoad.IsNone())
    {
        UGameplayStatics::OpenLevel(this, LevelToLoad);
    }
}