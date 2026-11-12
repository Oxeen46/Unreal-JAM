#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "EnemyAIController.generated.h"

class AEnemyCharacter;

UCLASS()
class EPICAIMEGAJAM_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;

	// Blackboard keys (must match the Blackboard asset)
	static const FName TargetActorKey;
	static const FName PatrolLocationKey;
	static const FName HomeLocationKey;
	static const FName LastKnownLocationKey;
	static const FName HasSeenPlayerKey;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* AIPerceptionComp;

	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY()
	AEnemyCharacter* ControlledEnemy;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void SetBlackboardTarget(AActor* Target);
	void ClearBlackboardTarget();
};