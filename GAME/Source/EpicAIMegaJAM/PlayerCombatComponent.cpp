
#include "PlayerCombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

UPlayerCombatComponent::UPlayerCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerCombatComponent::StartBlocking()
{
	if (bIsBlocking)
	{
		return;
	}

	bIsBlocking = true;
	BlockStartedAt = GetWorld()->GetTimeSeconds();
}

void UPlayerCombatComponent::StopBlocking()
{
	bIsBlocking = false;
	BlockStartedAt = -1.0f;
}

float UPlayerCombatComponent::ReceiveCombatHit(float BaseDamage, AActor* DamageCauser)
{
	if (BaseDamage <= 0.0f)
	{
		return 0.0f;
	}

	float FinalDamage = BaseDamage;

	if (bIsBlocking)
	{
		const float CurrentTime = GetWorld()->GetTimeSeconds();
		const float BlockDuration = CurrentTime - BlockStartedAt;

		if (BlockDuration >= 0.0f &&
			BlockDuration <= PerfectBlockWindow)
		{
			FinalDamage = 0.0f;
			OnPerfectBlock(DamageCauser);
		}
		else
		{
			FinalDamage *= (1.0f - BlockDamageReduction);
			OnNormalBlock(DamageCauser, FinalDamage);
		}
	}
	else
	{
		OnHitReceived(DamageCauser, FinalDamage);
	}

	if (FinalDamage > 0.0f)
	{
		AActor* OwnerActor = GetOwner();

		UGameplayStatics::ApplyDamage(
			OwnerActor,
			FinalDamage,
			DamageCauser ? DamageCauser->GetInstigatorController() : nullptr,
			DamageCauser,
			UDamageType::StaticClass()
		);
	}

	return FinalDamage;
}
