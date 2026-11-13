#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCombatComponent.generated.h"


UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class EPICAIMEGAJAM_API UPlayerCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerCombatComponent();

	// Hook this to your attack input
	UFUNCTION(BlueprintCallable, Category="Combat")
	void TryLightAttack();

	// Called by enemies
	UFUNCTION(BlueprintCallable, Category="Combat")
	void ReceiveCombatHit(float Damage, AActor* DamageInstigator);

	UFUNCTION(BlueprintPure, Category="Combat")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category="Combat")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category="Combat")
	bool IsAttacking() const { return bIsAttacking; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Health")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Health")
	float HitInvulnerableTime = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Attack")
	float AttackDamage = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Attack")
	float AttackRange = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Attack")
	float AttackRadius = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Timing")
	float WindUpTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Timing")
	float RecoveryTime = 0.45f;

	// Shows the swing's sweep and your health on screen while testing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Debug")
	bool bDebug = true;
	
	// Shown while the game reloads. Put your film reel animation in this widget.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Death")
	TSubclassOf<UUserWidget> DeathScreenClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Death")
	float DeathFadeTime = 1.2f;

	// How long the death screen stays up before reloading. Match your animation length.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Death")
	float DeathScreenTime = 4.f;

	UFUNCTION(BlueprintCallable, Category="Combat")
	void Heal(float Amount);
	
protected:
	virtual void BeginPlay() override;

	// Hooks for animation, sound and camera effects (implement in Blueprint)
	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnSwingStarted();

	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnSwingHit(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnSwingMissed();

	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnDamaged(float Damage, AActor* Source);

	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnDied();

private:
	void ExecuteSwing();
	void FinishSwing();

	float Health = 100.f;
	float InvulnerableUntil = 0.f;
	bool bIsAttacking = false;
	bool bIsDead = false;

	FTimerHandle SwingTimer;
	
	void BeginDeathSequence();
	void ShowDeathScreen();
	void Respawn();

	FTimerHandle DeathTimer;
};