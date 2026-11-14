
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyCombatComponent.generated.h"

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class EPICAIMEGAJAM_API UEnemyCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyCombatComponent();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void StartAttack(AActor* Target);

	UFUNCTION(BlueprintCallable, Category="Combat")
	void CancelAttack();

	UFUNCTION(BlueprintPure, Category="Combat")
	bool IsAttacking() const { return bIsAttacking; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Attack")
	float AttackDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Attack")
	float AttackRange = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Attack")
	float AttackAngle = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Timing")
	float AttackStartup = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Timing")
	float AttackRecovery = 1.2f;

	UPROPERTY(BlueprintReadOnly, Category="Combat")
	bool bIsAttacking = false;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget = nullptr;

	FTimerHandle AttackTimer;

	void ExecuteAttack();
	void FinishAttack();

	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnAttackTelegraph(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnAttackConnected(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnAttackMissed();

	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnAttackFinished();
};
