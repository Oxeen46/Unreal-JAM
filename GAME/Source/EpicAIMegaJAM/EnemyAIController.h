#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "EnemyAIController.generated.h"

class AEnemyCharacter;
class UEnemyCombatComponent;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Patrol,
	Chase,
	Attack,
	Search
};

UCLASS()
class EPICAIMEGAJAM_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category="AI")
	EEnemyState GetState() const { return State; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Patrol")
	float PatrolWaitTime = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Search")
	float SearchTime = 6.f;

	// Distance at which the enemy stops running and starts attacking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Attack")
	float AttackStartDistance = 160.f;

	// Pause between swings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Attack")
	float AttackCooldown = 0.8f;

	// How fast it turns toward you while attacking. Lower = easier to sidestep.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Attack")
	float TurnSpeed = 3.f;

	// Shows state changes on screen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Debug")
	bool bDebug = true;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY()
	TObjectPtr<AEnemyCharacter> ControlledEnemy;

	UPROPERTY()
	TObjectPtr<UEnemyCombatComponent> Combat;

	UPROPERTY()
	TObjectPtr<AActor> Target;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	void SetState(EEnemyState NewState);
	void EnterState(EEnemyState NewState);

	void TickPatrol(float DeltaTime);
	void TickChase(float DeltaTime);
	void TickAttack(float DeltaTime);
	void TickSearch(float DeltaTime);

	bool PickPatrolPoint(FVector& OutPoint) const;

	EEnemyState State = EEnemyState::Patrol;
	FVector LastKnownLocation = FVector::ZeroVector;
	float StateTimer = 0.f;
};