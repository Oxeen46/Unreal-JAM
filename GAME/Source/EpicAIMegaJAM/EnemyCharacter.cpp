#include "EnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/Engine.h"
#include "EnemyAIController.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Heavy, deliberate movement
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
	GetCharacterMovement()->MaxAcceleration = 280.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 500.f;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 140.f, 0.f);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	HomeLocation = GetActorLocation();
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
}

void AEnemyCharacter::SetChaseMode(bool bChase)
{
	bIsChasing = bChase;
	GetCharacterMovement()->MaxWalkSpeed = bChase ? ChaseSpeed : PatrolSpeed;
}
void AEnemyCharacter::ReceiveMeleeHit(float Damage, AActor* DamageInstigator)
{
	if (bIsDead) return;

	Health = FMath::Max(0.f, Health - Damage);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange,
			FString::Printf(TEXT("Enemy HP: %.0f"), Health));
	}

	// A hit interrupts the enemy's own attack
	if (UEnemyCombatComponent* Combat = FindComponentByClass<UEnemyCombatComponent>())
	{
		Combat->CancelAttack();
	}

	if (DamageInstigator)
	{
		const FVector Away = (GetActorLocation() - DamageInstigator->GetActorLocation()).GetSafeNormal2D();
		LaunchCharacter(Away * KnockbackStrength, true, false);
	}

	if (Health <= 0.f)
	{
		Die();
		return;
	}
	OnHitReaction(Damage);
}

void AEnemyCharacter::Die()
{
	bIsDead = true;

	if (AController* C = GetController())
	{
		C->UnPossess();
	}

	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);

	OnDeath();
	SetLifeSpan(10.f);
}