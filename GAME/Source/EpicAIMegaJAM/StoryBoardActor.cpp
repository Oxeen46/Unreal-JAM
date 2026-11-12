#include "StoryBoardActor.h"
#include "StoryBoardData.h"
#include "StoryStateSubsystem.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "InputCoreTypes.h"
#include "TimerManager.h"

namespace
{
	// All positions are in the board's local space: X = out of the board, Y = sideways, Z = up
	constexpr float SlotSpacing = 90.f;
	constexpr float SlotX = 2.f;
	constexpr float SlotZ = 30.f;
	constexpr float NoteX = 6.f;
	constexpr float TrayZ = -50.f;
	constexpr float NoteSpacing = 75.f;
	constexpr float DragX = 14.f;
	constexpr float SnapDistance = 55.f;

	FString BoardShortName(const FGameplayTag& Tag)
	{
		const FString Full = Tag.ToString();
		int32 Idx = INDEX_NONE;
		return Full.FindLastChar('.', Idx) ? Full.Mid(Idx + 1) : Full;
	}

	FVector SlotLocal(int32 Index, int32 Count)
	{
		return FVector(SlotX, (Index - (Count - 1) * 0.5f) * SlotSpacing, SlotZ);
	}

	FVector TrayLocal(int32 Index, int32 Count)
	{
		return FVector(NoteX, (Index - (Count - 1) * 0.5f) * NoteSpacing, TrayZ);
	}

	void Tint(UStaticMeshComponent* Mesh, UMaterialInterface* Mat, const FLinearColor& Color)
	{
		if (!Mesh || !Mat) return;
		if (UMaterialInstanceDynamic* MID = Mesh->CreateDynamicMaterialInstance(0, Mat))
		{
			MID->SetVectorParameterValue(TEXT("Color"), Color);
		}
	}

	void StyleText(UTextRenderComponent* Text, float Size, FColor Color)
	{
		Text->SetHorizontalAlignment(EHTA_Center);
		Text->SetVerticalAlignment(EVRTA_TextCenter);
		Text->SetWorldSize(Size);
		Text->SetTextRenderColor(Color);
	}
}

AStoryBoardActor::AStoryBoardActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (CubeFinder.Succeeded()) CubeMesh = CubeFinder.Object;
	if (MatFinder.Succeeded()) BasicMat = MatFinder.Object;

	BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
	BoardMesh->SetupAttachment(SceneRoot);
	BoardMesh->SetStaticMesh(CubeMesh);
	BoardMesh->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	BoardMesh->SetRelativeScale3D(FVector(0.02f, 6.f, 2.2f));
	BoardMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	// Sits in front of the board (+X) looking back at it
	BoardCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BoardCamera"));
	BoardCamera->SetupAttachment(SceneRoot);
	BoardCamera->SetRelativeLocation(FVector(360.f, 0.f, 10.f));
	BoardCamera->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));

	TitleText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TitleText"));
	TitleText->SetupAttachment(SceneRoot);
	TitleText->SetRelativeLocation(FVector(3.f, 0.f, 95.f));
	StyleText(TitleText, 12.f, FColor(235, 225, 205));
	TitleText->SetText(FText::GetEmpty());

	StatusText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusText"));
	StatusText->SetupAttachment(SceneRoot);
	StatusText->SetRelativeLocation(FVector(3.f, 0.f, -10.f));
	StyleText(StatusText, 9.f, FColor(220, 70, 55));
	StatusText->SetText(FText::GetEmpty());

	HintText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HintText"));
	HintText->SetupAttachment(SceneRoot);
	HintText->SetRelativeLocation(FVector(3.f, 0.f, -88.f));
	StyleText(HintText, 5.f, FColor(160, 150, 130));
	HintText->SetText(FText::FromString(TEXT("Drag notes onto the board   |   Enter: Rewrite   |   Backspace: Leave")));
	HintText->SetHiddenInGame(true);
}

void AStoryBoardActor::BeginPlay()
{
	Super::BeginPlay();
	BuildSlots();
}

UStoryStateSubsystem* AStoryBoardActor::GetStory() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr;
}

void AStoryBoardActor::BuildSlots()
{
	if (!Board) return;

	const int32 Count = Board->Slots.Num();
	BoardWidth = FMath::Max(6, Count) * SlotSpacing + 60.f;

	BoardMesh->SetRelativeScale3D(FVector(0.02f, BoardWidth / 100.f, 2.2f));
	BoardCamera->SetRelativeLocation(FVector(BoardWidth * 0.5f + 60.f, 0.f, 10.f));
	Tint(BoardMesh, BasicMat, FLinearColor(0.06f, 0.055f, 0.05f));

	TitleText->SetText(Board->SceneTitle);

	for (int32 i = 0; i < Count; ++i)
	{
		FBoardSlotVisual Visual;

		Visual.Mesh = NewObject<UStaticMeshComponent>(this);
		Visual.Mesh->SetStaticMesh(CubeMesh);
		Visual.Mesh->SetupAttachment(SceneRoot);
		Visual.Mesh->RegisterComponent();
		Visual.Mesh->SetRelativeLocation(SlotLocal(i, Count));
		Visual.Mesh->SetRelativeScale3D(FVector(0.01f, 0.7f, 0.45f));
		Visual.Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Tint(Visual.Mesh, BasicMat, FLinearColor(0.15f, 0.14f, 0.13f));

		Visual.Text = NewObject<UTextRenderComponent>(this);
		Visual.Text->SetupAttachment(SceneRoot);
		Visual.Text->RegisterComponent();
		Visual.Text->SetRelativeLocation(SlotLocal(i, Count) + FVector(1.f, 0.f, 30.f));
		StyleText(Visual.Text, 5.f, FColor(200, 190, 170));
		Visual.Text->SetText(Board->Slots[i].Label);

		SlotVisuals.Add(Visual);
	}
}

void AStoryBoardActor::BuildNotes()
{
	DestroyNotes();

	UStoryStateSubsystem* Story = GetStory();
	if (!Story) return;

	for (const FGameplayTag& Clue : Story->GetCollectedClues())
	{
		FBoardNote Note;
		Note.Clue = Clue;

		Note.Root = NewObject<USceneComponent>(this);
		Note.Root->SetupAttachment(SceneRoot);
		Note.Root->RegisterComponent();

		Note.Mesh = NewObject<UStaticMeshComponent>(this);
		Note.Mesh->SetStaticMesh(CubeMesh);
		Note.Mesh->SetupAttachment(Note.Root);
		Note.Mesh->RegisterComponent();
		Note.Mesh->SetRelativeScale3D(FVector(0.015f, 0.55f, 0.38f));
		Note.Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Note.Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		Note.Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		Tint(Note.Mesh, BasicMat, FLinearColor(0.96f, 0.84f, 0.43f));

		Note.Text = NewObject<UTextRenderComponent>(this);
		Note.Text->SetupAttachment(Note.Root);
		Note.Text->RegisterComponent();
		Note.Text->SetRelativeLocation(FVector(1.f, 0.f, 0.f));
		StyleText(Note.Text, 7.f, FColor(30, 25, 15));
		Note.Text->SetText(FText::FromString(BoardShortName(Clue)));

		Notes.Add(Note);
	}
}

void AStoryBoardActor::DestroyNotes()
{
	for (FBoardNote& Note : Notes)
	{
		if (Note.Text) Note.Text->DestroyComponent();
		if (Note.Mesh) Note.Mesh->DestroyComponent();
		if (Note.Root) Note.Root->DestroyComponent();
	}
	Notes.Empty();
	DraggedIndex = INDEX_NONE;
}

void AStoryBoardActor::RefreshLayout()
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Story || !Board) return;

	const int32 SlotCount = Board->Slots.Num();
	TArray<int32> Unplaced;

	for (int32 n = 0; n < Notes.Num(); ++n)
	{
		if (n == DraggedIndex) continue;

		int32 SlotIndex = INDEX_NONE;
		for (int32 s = 0; s < SlotCount; ++s)
		{
			if (Story->GetPlacedClue(Board->Slots[s].SlotTag) == Notes[n].Clue)
			{
				SlotIndex = s;
				break;
			}
		}

		if (SlotIndex != INDEX_NONE)
		{
			Notes[n].Root->SetRelativeLocation(SlotLocal(SlotIndex, SlotCount) + FVector(NoteX - SlotX, 0.f, 0.f));
		}
		else
		{
			Unplaced.Add(n);
		}
	}

	for (int32 k = 0; k < Unplaced.Num(); ++k)
	{
		Notes[Unplaced[k]].Root->SetRelativeLocation(TrayLocal(k, Unplaced.Num()));
	}
}

FText AStoryBoardActor::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return (bCompleted || bActive) ? FText::GetEmpty() : InteractionText;
}

void AStoryBoardActor::Interact_Implementation(AActor* Interactor)
{
	if (bActive || ActivePC || !Board) return;

	if (bCompleted)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::White, TEXT("This story is already written."));
		}
		return;
	}

	APawn* Pawn = Cast<APawn>(Interactor);
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (PC)
	{
		EnterBoard(PC);
	}
}

void AStoryBoardActor::EnterBoard(APlayerController* PC)
{
	ActivePC = PC;
	bActive = true;

	BuildNotes();
	RefreshLayout();
	StatusText->SetText(FText::GetEmpty());
	HintText->SetHiddenInGame(false);

	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);
	PC->SetViewTargetWithBlend(this, BlendTime, VTBlend_Cubic);

	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(Mode);
	PC->SetShowMouseCursor(true);

	bPrevLMB = PC->IsInputKeyDown(EKeys::LeftMouseButton);
	bPrevEnter = PC->IsInputKeyDown(EKeys::Enter);
	bPrevBack = PC->IsInputKeyDown(EKeys::BackSpace);
	InputReadyTime = GetWorld()->GetTimeSeconds() + BlendTime;

	SetActorTickEnabled(true);
}

void AStoryBoardActor::ExitBoard()
{
	if (!bActive || !ActivePC) return;

	bActive = false;
	DraggedIndex = INDEX_NONE;
	StatusText->SetText(FText::GetEmpty());

	ActivePC->SetViewTargetWithBlend(ActivePC->GetPawn(), BlendTime, VTBlend_Cubic);
	GetWorldTimerManager().SetTimer(ExitTimer, this, &AStoryBoardActor::FinishExit, BlendTime, false);
}

void AStoryBoardActor::FinishExit()
{
	if (ActivePC)
	{
		ActivePC->SetIgnoreMoveInput(false);
		ActivePC->SetIgnoreLookInput(false);
		ActivePC->SetInputMode(FInputModeGameOnly());
		ActivePC->SetShowMouseCursor(false);
		ActivePC = nullptr;
	}
	DestroyNotes();
	HintText->SetHiddenInGame(true);
	SetActorTickEnabled(false);
}

void AStoryBoardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bActive || !ActivePC) return;

	const bool bLMB = ActivePC->IsInputKeyDown(EKeys::LeftMouseButton);
	const bool bEnter = ActivePC->IsInputKeyDown(EKeys::Enter);
	const bool bBack = ActivePC->IsInputKeyDown(EKeys::BackSpace);

	const bool bLMBPressed = bLMB && !bPrevLMB;
	const bool bLMBReleased = !bLMB && bPrevLMB;
	const bool bEnterPressed = bEnter && !bPrevEnter;
	const bool bBackPressed = bBack && !bPrevBack;

	bPrevLMB = bLMB;
	bPrevEnter = bEnter;
	bPrevBack = bBack;

	// Ignore input while the camera is still blending in
	if (GetWorld()->GetTimeSeconds() < InputReadyTime) return;

	if (bBackPressed)
	{
		ExitBoard();
		return;
	}

	if (bEnterPressed)
	{
		TryRewrite();
		if (!bActive) return;
	}

	if (bLMBPressed) BeginDrag();
	if (bLMB && DraggedIndex != INDEX_NONE) UpdateDrag();
	if (bLMBReleased && DraggedIndex != INDEX_NONE) EndDrag();
}

void AStoryBoardActor::BeginDrag()
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Story || !Board) return;

	FHitResult Hit;
	ActivePC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	UPrimitiveComponent* Clicked = Hit.GetComponent();
	if (!Clicked) return;

	for (int32 i = 0; i < Notes.Num(); ++i)
	{
		if (Notes[i].Mesh == Clicked)
		{
			DraggedIndex = i;
			StatusText->SetText(FText::GetEmpty());

			// Picking a note up removes it from whatever slot it was in
			for (const FStoryBoardSlot& BoardSlot : Board->Slots)
			{
				if (Story->GetPlacedClue(BoardSlot.SlotTag) == Notes[i].Clue)
				{
					Story->ClearSlot(BoardSlot.SlotTag);
				}
			}
			RefreshLayout();
			return;
		}
	}
}

void AStoryBoardActor::UpdateDrag()
{
	if (!Notes.IsValidIndex(DraggedIndex)) return;

	FVector Origin, Direction;
	if (!ActivePC->DeprojectMousePositionToWorld(Origin, Direction)) return;

	const FTransform BoardTransform = GetActorTransform();
	const FVector PlanePoint = BoardTransform.TransformPosition(FVector(DragX, 0.f, 0.f));
	const FVector PlaneNormal = GetActorForwardVector();
	const FVector HitPoint = FMath::LinePlaneIntersection(Origin, Origin + Direction * 20000.f, PlanePoint, PlaneNormal);

	FVector Local = BoardTransform.InverseTransformPosition(HitPoint);
	Local.X = DragX;
	Local.Y = FMath::Clamp(Local.Y, -BoardWidth * 0.5f + 30.f, BoardWidth * 0.5f - 30.f);
	Local.Z = FMath::Clamp(Local.Z, -85.f, 100.f);

	Notes[DraggedIndex].Root->SetRelativeLocation(Local);
}

void AStoryBoardActor::EndDrag()
{
	UStoryStateSubsystem* Story = GetStory();
	if (Story && Board && Notes.IsValidIndex(DraggedIndex))
	{
		const FVector Local = Notes[DraggedIndex].Root->GetRelativeLocation();
		const int32 Count = Board->Slots.Num();

		int32 Best = INDEX_NONE;
		double BestDistance = SnapDistance;
		for (int32 i = 0; i < Count; ++i)
		{
			const FVector SlotPos = SlotLocal(i, Count);
			const double Distance = FVector2D::Distance(FVector2D(Local.Y, Local.Z), FVector2D(SlotPos.Y, SlotPos.Z));
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				Best = i;
			}
		}

		if (Best != INDEX_NONE)
		{
			Story->PlaceClue(Board->Slots[Best].SlotTag, Notes[DraggedIndex].Clue);
		}
	}

	DraggedIndex = INDEX_NONE;
	RefreshLayout();
}

void AStoryBoardActor::TryRewrite()
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Story || !Board) return;

	const int32 Index = Story->CommitBoard(Board);
	if (!Board->Storylines.IsValidIndex(Index))
	{
		StatusText->SetTextRenderColor(FColor(220, 70, 55));
		StatusText->SetText(FText::FromString(TEXT("That doesn't fit.")));
		return;
	}

	StatusText->SetTextRenderColor(FColor(120, 200, 120));
	StatusText->SetText(Board->Storylines[Index].Title);

	if (bLockAfterCommit)
	{
		bCompleted = true;
	}
	ExitBoard();
}