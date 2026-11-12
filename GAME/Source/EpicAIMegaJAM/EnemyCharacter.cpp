#include "EnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	HomeLocation = GetActorLocation();
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
}

void AEnemyCharacter::SetChaseMode(bool bChase)
{
	bIsChasing = bChase;
	GetCharacterMovement()->MaxWalkSpeed = bChase ? ChaseSpeed : PatrolSpeed;
}