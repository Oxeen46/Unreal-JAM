#include "PlayerCombatComponent.h"
#include "EnemyCharacter.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "StoryStateSubsystem.h"
#include "InventoryComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "GameSettingsSubsystem.h"


UPlayerCombatComponent::UPlayerCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;

	// If we got here by respawning, move the player to the checkpoint
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UStoryStateSubsystem* Story = GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr;

	FTransform Spawn;
	if (Story && Story->ConsumePendingRespawn(Spawn))
	{
		TWeakObjectPtr<UPlayerCombatComponent> Weak(this);
		GetWorld()->GetTimerManager().SetTimerForNextTick([Weak, Spawn]()
		{
			UPlayerCombatComponent* Self = Weak.Get();
			APawn* Pawn = Self ? Cast<APawn>(Self->GetOwner()) : nullptr;
			if (!Pawn) return;

			Pawn->SetActorLocationAndRotation(Spawn.GetLocation(), Spawn.GetRotation(), false, nullptr, ETeleportType::TeleportPhysics);

			if (AController* Controller = Pawn->GetController())
			{
				Controller->SetControlRotation(Spawn.Rotator());
			}
			if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
			{
				if (PC->PlayerCameraManager)
				{
					PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, 1.5f, FLinearColor::Black, false, false);
				}
			}
		});
	}
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UGameSettingsSubsystem* Settings = GameInstance->GetSubsystem<UGameSettingsSubsystem>())
		{
			Settings->ApplyAll();
		}
	}
}

void UPlayerCombatComponent::TryLightAttack()
{
	if (bIsDead || bIsAttacking) return;

	const APawn* Pawn = Cast<APawn>(GetOwner());
	const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC) return;

	// No swinging while a menu or the storyboard is open (cursor visible)
	if (PC->bShowMouseCursor) return;

	bIsAttacking = true;
	OnSwingStarted();

	GetWorld()->GetTimerManager().SetTimer(SwingTimer, this, &UPlayerCombatComponent::ExecuteSwing, WindUpTime, false);
}

void UPlayerCombatComponent::ExecuteSwing()
{
	AActor* Owner = GetOwner();
	const APawn* Pawn = Cast<APawn>(Owner);
	const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || bIsDead)
	{
		FinishSwing();
		return;
	}

	FVector Start;
	FRotator Rotation;
	PC->GetPlayerViewPoint(Start, Rotation);
	const FVector End = Start + Rotation.Vector() * AttackRange;

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PlayerMelee), false, Owner);
	GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(AttackRadius), Params);

	if (bDebug)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.5f);
		DrawDebugSphere(GetWorld(), End, AttackRadius, 12, FColor::Red, false, 0.5f);
	}

	TSet<AActor*> Already;
	bool bHitAny = false;

	for (const FHitResult& Hit : Hits)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Hit.GetActor());
		if (!Enemy || Already.Contains(Enemy) || Enemy->IsDead()) continue;

		Already.Add(Enemy);
		Enemy->ReceiveMeleeHit(AttackDamage, Owner);
		OnSwingHit(Enemy);
		bHitAny = true;
	}

	if (bHitAny)
	{
		if (UInventoryComponent* Inventory = Owner->FindComponentByClass<UInventoryComponent>())
		{
			Inventory->OnWeaponHit();
		}
	}
	else
	{
		OnSwingMissed();
	}

	GetWorld()->GetTimerManager().SetTimer(SwingTimer, this, &UPlayerCombatComponent::FinishSwing, RecoveryTime, false);
}

void UPlayerCombatComponent::FinishSwing()
{
	bIsAttacking = false;
}

void UPlayerCombatComponent::ReceiveCombatHit(float Damage, AActor* DamageInstigator)
{
	if (bIsDead) return;

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now < InvulnerableUntil) return;
	InvulnerableUntil = Now + HitInvulnerableTime;

	Health = FMath::Max(0.f, Health - Damage);

	if (bDebug && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
			FString::Printf(TEXT("Player HP: %.0f"), Health));
	}

	OnDamaged(Damage, DamageInstigator);

	if (Health <= 0.f)
	{
		bIsDead = true;
		GetWorld()->GetTimerManager().ClearTimer(SwingTimer);
		bIsAttacking = false;
		OnDied();
		BeginDeathSequence();
	}
}
void UPlayerCombatComponent::BeginDeathSequence()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (PC)
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, DeathFadeTime, FLinearColor::Black, false, true);
		}
	}
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &UPlayerCombatComponent::ShowDeathScreen, DeathFadeTime, false);
}

void UPlayerCombatComponent::ShowDeathScreen()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (PC && DeathScreenClass)
	{
		if (UUserWidget* Widget = CreateWidget<UUserWidget>(PC, DeathScreenClass))
		{
			Widget->AddToViewport(100);
		}
	}
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &UPlayerCombatComponent::Respawn, DeathScreenTime, false);
}

void UPlayerCombatComponent::Respawn()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (UStoryStateSubsystem* Story = GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr)
	{
		Story->RespawnFromCheckpoint();
	}
}
void UPlayerCombatComponent::Heal(float Amount)
{
	if (bIsDead) return;

	Health = FMath::Min(MaxHealth, Health + Amount);

	if (bDebug && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
			FString::Printf(TEXT("Player HP: %.0f"), Health));
	}
}