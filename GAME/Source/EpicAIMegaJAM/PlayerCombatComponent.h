
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

	UFUNCTION(BlueprintCallable, Category="Combat")
	void StartBlocking();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void StopBlocking();

	UFUNCTION(BlueprintCallable, Category="Combat")
	float ReceiveCombatHit(float BaseDamage, AActor* DamageCauser);

	UFUNCTION(BlueprintPure, Category="Combat")
	bool IsBlocking() const { return bIsBlocking; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Blocking")
	float PerfectBlockWindow = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Blocking")
	float BlockDamageReduction = 0.6f;

	UPROPERTY(BlueprintReadOnly, Category="Combat|Blocking")
	bool bIsBlocking = false;

	float BlockStartedAt = -1.0f;

	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnPerfectBlock(AActor* Attacker);

	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnNormalBlock(AActor* Attacker, float RemainingDamage);

	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnHitReceived(AActor* Attacker, float Damage);
};
