#include "InteractionComponent.h"
#include "InteractableInterface.h"
#include "InteractionPromptWidget.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
}

APlayerController* UInteractionComponent::GetPC() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	return Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
}

AActor* UInteractionComponent::FindTarget() const
{
	APlayerController* PC = GetPC();
	if (!PC) return nullptr;

	FVector Loc;
	FRotator Rot;
	PC->GetPlayerViewPoint(Loc, Rot);
	const FVector End = Loc + Rot.Vector() * InteractDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(Interact), false, GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit, Loc, End, ECC_Visibility, Params))
	{
		AActor* Target = Hit.GetActor();
		if (Target && Target->Implements<UInteractableInterface>())
		{
			return Target;
		}
	}
	return nullptr;
}

void UInteractionComponent::TryInteract()
{
	if (AActor* Target = FindTarget())
	{
		IInteractableInterface::Execute_Interact(Target, GetOwner());
	}
}

FText UInteractionComponent::GetKeyText() const
{
	APlayerController* PC = GetPC();
	ULocalPlayer* LP = PC ? PC->GetLocalPlayer() : nullptr;

	if (InteractAction && LP)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Input = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			const TArray<FKey> Keys = Input->QueryKeysMappedToAction(InteractAction);
			if (Keys.Num() > 0)
			{
				return Keys[0].GetDisplayName();
			}
		}
	}
	return FText::FromString(TEXT("E"));
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APlayerController* PC = GetPC();
	if (!PC || !PC->IsLocalController() || !PromptWidgetClass) return;

	// Create the prompt the first time we have a player controller
	if (!PromptWidget)
	{
		PromptWidget = CreateWidget<UInteractionPromptWidget>(PC, PromptWidgetClass);
		if (!PromptWidget) return;
		PromptWidget->AddToViewport();
		PromptWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Hide while a menu is open (mouse cursor showing)
	AActor* Target = PC->bShowMouseCursor ? nullptr : FindTarget();

	FText Prompt;
	if (Target)
	{
		Prompt = IInteractableInterface::Execute_GetInteractionPrompt(Target, GetOwner());
	}

	if (Prompt.IsEmpty())
	{
		PromptWidget->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	PromptWidget->SetPrompt(GetKeyText(), Prompt);
	PromptWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
}