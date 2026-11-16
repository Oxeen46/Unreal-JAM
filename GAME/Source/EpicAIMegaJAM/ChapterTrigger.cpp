#include "ChapterTrigger.h"
#include "ChapterTitleWidget.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AChapterTrigger::AChapterTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    SetRootComponent(TriggerBox);

    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerBox->SetGenerateOverlapEvents(true);
}

void AChapterTrigger::BeginPlay()
{
    Super::BeginPlay();

    TriggerBox->OnComponentBeginOverlap.AddDynamic(
        this,
        &AChapterTrigger::OnPlayerEnter
    );
}

void AChapterTrigger::OnPlayerEnter(
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

    if (!PC || !ChapterWidgetClass)
        return;

    ActiveWidget = CreateWidget<UChapterTitleWidget>(
        PC,
        ChapterWidgetClass
    );

    if (ActiveWidget)
    {
        ActiveWidget->SetChapterTitle(ChapterTitle);
        ActiveWidget->AddToViewport(100);
        ActiveWidget->PlayChapterAnimation();

        GetWorldTimerManager().SetTimer(
            RemoveWidgetTimer,
            this,
            &AChapterTrigger::RemoveChapterWidget,
            DisplayDuration,
            false
        );
    }
}

void AChapterTrigger::RemoveChapterWidget()
{
    if (ActiveWidget)
    {
        ActiveWidget->RemoveFromParent();
        ActiveWidget = nullptr;
    }

    if (!bTriggerOnce)
    {
        bTriggered = false;
    }
}