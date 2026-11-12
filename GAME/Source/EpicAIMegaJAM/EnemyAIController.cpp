#include "EnemyAIController.h"
#include "EnemyCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "GameFramework/Character.h"

const FName AEnemyAIController::TargetActorKey = TEXT("TargetActor");
const FName AEnemyAIController::PatrolLocationKey = TEXT("PatrolLocation");
const FName AEnemyAIController::HomeLocationKey = TEXT("HomeLocation");
const FName AEnemyAIController::LastKnownLocationKey = TEXT("LastKnownLocation");
const FName AEnemyAIController::HasSeenPlayerKey = TEXT("HasSeenPlayer");

AEnemyAIController::AEnemyAIController()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerceptionComp);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1400.f;
	SightConfig->LoseSightRadius = 1900.f;
	SightConfig->PeripheralVisionAngleDegrees = 80.f;		// narrower = more focused / creepy
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.f;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledEnemy = Cast<AEnemyCharacter>(InPawn);
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);
	}

	// Run your Behavior Tree here (or set it in the Character defaults)
	// RunBehaviorTree(EnemyBehaviorTree);
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !ControlledEnemy) return;

	// Only react to the player (adjust tag / class as needed)
	if (!Actor->ActorHasTag(TEXT("Player"))) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		// First time seeing the player → Silent Hill "notice" moment
		if (!GetBlackboardComponent()->GetValueAsBool(HasSeenPlayerKey))
		{
			// You can play a montage / sound here via the Character
		}

		SetBlackboardTarget(Actor);
		ControlledEnemy->SetChaseMode(true);
	}
	else
	{
		// Lost sight – keep last known location for a while
		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			BB->SetValueAsVector(LastKnownLocationKey, Stimulus.StimulusLocation);
		}

		// Optional: clear target after a delay (use a timer for true memory)
		// For now we clear immediately and let the BT handle LastKnownLocation
		ClearBlackboardTarget();
		ControlledEnemy->SetChaseMode(false);
	}
}

void AEnemyAIController::SetBlackboardTarget(AActor* Target)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(TargetActorKey, Target);
		BB->SetValueAsBool(HasSeenPlayerKey, true);
		BB->SetValueAsVector(LastKnownLocationKey, Target->GetActorLocation());
	}
}

void AEnemyAIController::ClearBlackboardTarget()
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->ClearValue(TargetActorKey);
	}
}