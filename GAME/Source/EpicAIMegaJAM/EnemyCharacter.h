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

protected:
	bool bIsChasing = false;
};