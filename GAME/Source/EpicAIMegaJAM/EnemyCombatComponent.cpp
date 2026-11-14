
#include "EnemyCombatComponent.h"
#include "PlayerCombatComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UEnemyCombatComponent::UEnemyCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyCombatComponent::StartAttack(AActor* Target)
{
    if (bIsAttacking || !IsValid(Target))
    {
        return;
    }

    bIsAttacking = true;
    CurrentTarget = Target;

    OnAttackTelegraph(CurrentTarget);

    GetWorld()->GetTimerManager().SetTimer(
        AttackTimer,
        this,
        &UEnemyCombatComponent::ExecuteAttack,
        AttackStartup,
        false
    );
}

void UEnemyCombatComponent::ExecuteAttack()
{
    if (!IsValid(CurrentTarget))
    {
        FinishAttack();
        return;
    }

    AActor* OwnerActor = GetOwner();

    if (!IsValid(OwnerActor))
    {
        FinishAttack();
        return;
    }

    const FVector Direction =
        CurrentTarget->GetActorLocation() - OwnerActor->GetActorLocation();

    const float DistanceSquared = Direction.SizeSquared();

    const float MaxRangeSquared = AttackRange * AttackRange;

    const float DotProduct = FVector::DotProduct(
        OwnerActor->GetActorForwardVector(),
        Direction.GetSafeNormal()
    );

    const float MinimumDot = FMath::Cos(
        FMath::DegreesToRadians(AttackAngle * 0.5f)
    );

    if (DistanceSquared <= MaxRangeSquared && DotProduct >= MinimumDot)
    {
        UPlayerCombatComponent* CombatComponent =
            CurrentTarget->FindComponentByClass<UPlayerCombatComponent>();

        if (CombatComponent)
        {
            CombatComponent->ReceiveCombatHit(AttackDamage, OwnerActor);
            OnAttackConnected(CurrentTarget);
        }
        else
        {
            OnAttackMissed();
        }
    }
    else
    {
        OnAttackMissed();
    }

    GetWorld()->GetTimerManager().SetTimer(
        AttackTimer,
        this,
        &UEnemyCombatComponent::FinishAttack,
        AttackRecovery,
        false
    );
}

void UEnemyCombatComponent::FinishAttack()
{
    bIsAttacking = false;
    CurrentTarget = nullptr;

    GetWorld()->GetTimerManager().ClearTimer(AttackTimer);

    OnAttackFinished();
}

void UEnemyCombatComponent::CancelAttack()
{
    GetWorld()->GetTimerManager().ClearTimer(AttackTimer);

    bIsAttacking = false;
    CurrentTarget = nullptr;

    OnAttackFinished();
}
