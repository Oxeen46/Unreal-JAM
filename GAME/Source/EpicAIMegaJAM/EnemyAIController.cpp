#include "EnemyAIController.h"
#include "EnemyCharacter.h"
#include "EnemyCombatComponent.h"
#include "PlayerCombatComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerceptionComp);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1400.f;
	SightConfig->LoseSightRadius = 1900.f;
	SightConfig->PeripheralVisionAngleDegrees = 70.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->AutoSuccessRangeFromLastSeenLocation = -1.f; // hiding works

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);
	}
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledEnemy = Cast<AEnemyCharacter>(InPawn);
	if (!ControlledEnemy) return;

	// Use the Blueprint's combat component if it has one, otherwise add one
	Combat = ControlledEnemy->FindComponentByClass<UEnemyCombatComponent>();
	if (!Combat)
	{
		Combat = NewObject<UEnemyCombatComponent>(ControlledEnemy);
		Combat->RegisterComponent();
	}

	Target = nullptr;
	EnterState(EEnemyState::Patrol);
}

void AEnemyAIController::OnUnPossess()
{
	StopMovement();
	ControlledEnemy = nullptr;
	Combat = nullptr;
	Target = nullptr;
	Super::OnUnPossess();
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !ControlledEnemy || ControlledEnemy->IsDead()) return;

	// Only react to the player
	const APawn* Other = Cast<APawn>(Actor);
	if (!Other || !Other->IsPlayerControlled()) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		Target = Actor;
		LastKnownLocation = Actor->GetActorLocation();
		if (State != EEnemyState::Attack)
		{
			SetState(EEnemyState::Chase);
		}
	}
	else if (Actor == Target)
	{
		LastKnownLocation = Stimulus.StimulusLocation;
		Target = nullptr;
		SetState(EEnemyState::Search);
	}
}

void AEnemyAIController::SetState(EEnemyState NewState)
{
	if (NewState == State) return;
	EnterState(NewState);
}

void AEnemyAIController::EnterState(EEnemyState NewState)
{
	if (!ControlledEnemy) return;

	State = NewState;
	StateTimer = 0.f;

	switch (State)
	{
	case EEnemyState::Patrol:
		ControlledEnemy->SetChaseMode(false);
		StopMovement();
		StateTimer = PatrolWaitTime; // pick a point right away
		break;

	case EEnemyState::Chase:
		ControlledEnemy->SetChaseMode(true);
		StateTimer = 10.f; // path to the target right away
		break;

	case EEnemyState::Attack:
		StopMovement();
		StateTimer = AttackCooldown; // swing right away
		break;

	case EEnemyState::Search:
		ControlledEnemy->SetChaseMode(false);
		MoveToLocation(LastKnownLocation, 60.f, true, true, true);
		break;
	}

	if (bDebug && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
			FString::Printf(TEXT("Enemy: %s"),
				*StaticEnum<EEnemyState>()->GetNameStringByValue(static_cast<int64>(State))));
	}
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!ControlledEnemy || ControlledEnemy->IsDead()) return;

	// Stop hunting a dead player
	if (Target)
	{
		if (const UPlayerCombatComponent* PlayerCombat = Target->FindComponentByClass<UPlayerCombatComponent>())
		{
			if (PlayerCombat->IsDead())
			{
				Target = nullptr;
				SetState(EEnemyState::Patrol);
			}
		}
	}

	switch (State)
	{
	case EEnemyState::Patrol: TickPatrol(DeltaTime); break;
	case EEnemyState::Chase:  TickChase(DeltaTime);  break;
	case EEnemyState::Attack: TickAttack(DeltaTime); break;
	case EEnemyState::Search: TickSearch(DeltaTime); break;
	}
}

bool AEnemyAIController::PickPatrolPoint(FVector& OutPoint) const
{
	UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!Nav || !ControlledEnemy) return false;

	FNavLocation Result;
	if (Nav->GetRandomReachablePointInRadius(ControlledEnemy->HomeLocation, ControlledEnemy->PatrolRadius, Result))
	{
		OutPoint = Result.Location;
		return true;
	}
	return false;
}

void AEnemyAIController::TickPatrol(float DeltaTime)
{
	if (GetMoveStatus() != EPathFollowingStatus::Idle) return; // still walking

	StateTimer += DeltaTime;
	if (StateTimer < PatrolWaitTime) return;
	StateTimer = 0.f;

	FVector Point;
	if (PickPatrolPoint(Point))
	{
		MoveToLocation(Point, 60.f);
	}
	else if (bDebug && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
			TEXT("Enemy can't find a patrol point. Is there a NavMeshBoundsVolume in the level?"));
	}
}

void AEnemyAIController::TickChase(float DeltaTime)
{
	APawn* Self = GetPawn();
	if (!Self || !IsValid(Target))
	{
		SetState(EEnemyState::Search);
		return;
	}

	const float Distance = FVector::Dist(Self->GetActorLocation(), Target->GetActorLocation());
	if (Distance <= AttackStartDistance)
	{
		SetState(EEnemyState::Attack);
		return;
	}

	StateTimer += DeltaTime;
	if (StateTimer >= 0.3f)
	{
		StateTimer = 0.f;
		MoveToActor(Target, AttackStartDistance * 0.7f);
	}
}

void AEnemyAIController::TickAttack(float DeltaTime)
{
	APawn* Self = GetPawn();
	if (!Self || !IsValid(Target))
	{
		SetState(EEnemyState::Search);
		return;
	}

	// Turn slowly toward the target so the player can sidestep a swing
	FRotator Look = (Target->GetActorLocation() - Self->GetActorLocation()).Rotation();
	Look.Pitch = 0.f;
	Look.Roll = 0.f;
	Self->SetActorRotation(FMath::RInterpTo(Self->GetActorRotation(), Look, DeltaTime, TurnSpeed));

	if (Combat && Combat->IsAttacking())
	{
		StateTimer = 0.f;
		return;
	}

	StateTimer += DeltaTime;
	if (StateTimer < AttackCooldown) return;

	const float Distance = FVector::Dist(Self->GetActorLocation(), Target->GetActorLocation());
	if (Distance > AttackStartDistance * 1.4f)
	{
		SetState(EEnemyState::Chase);
		return;
	}

	if (Combat)
	{
		Combat->StartAttack(Target);
		StateTimer = 0.f;
	}
}

void AEnemyAIController::TickSearch(float DeltaTime)
{
	if (GetMoveStatus() != EPathFollowingStatus::Idle) return; // still walking there

	StateTimer += DeltaTime;
	if (StateTimer >= SearchTime)
	{
		SetState(EEnemyState::Patrol);
	}
}