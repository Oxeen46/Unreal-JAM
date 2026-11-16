#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class EPICAIMEGAJAM_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	virtual void BeginPlay() override;

	// Designated area
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
	float PatrolRadius = 1200.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Patrol")
	FVector HomeLocation;

	// Speeds (Silent Hill feel)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Movement")
	float PatrolSpeed = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Movement")
	float ChaseSpeed = 210.f;

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetChaseMode(bool bChase);

	UFUNCTION(BlueprintCallable, Category = "AI")
	bool IsInChaseMode() const { return bIsChasing; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float KnockbackStrength = 350.f;

	UFUNCTION(BlueprintCallable, Category="Combat")
	void ReceiveMeleeHit(float Damage, AActor* DamageInstigator);

	UFUNCTION(BlueprintPure, Category="Combat")
	bool IsDead() const { return bIsDead; }

	// Animation / sound hooks
	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnHitReaction(float Damage);

	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnDeath();

protected:
	bool bIsChasing = false;
	bool bIsDead = false;
	void Die();
};