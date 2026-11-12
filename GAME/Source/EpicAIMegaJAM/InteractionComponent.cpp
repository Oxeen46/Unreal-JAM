#include "InteractionComponent.h"
#include "InteractableInterface.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

void UInteractionComponent::TryInteract()
{
	AActor* Owner = GetOwner();
	APawn* Pawn = Cast<APawn>(Owner);
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC) return;

	FVector Loc;
	FRotator Rot;
	PC->GetPlayerViewPoint(Loc, Rot);
	const FVector End = Loc + Rot.Vector() * InteractDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(Interact), false, Owner);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Loc, End, ECC_Visibility, Params))
	{
		AActor* Target = Hit.GetActor();
		if (Target && Target->Implements<UInteractableInterface>())
		{
			IInteractableInterface::Execute_Interact(Target, Owner);
		}
	}
}