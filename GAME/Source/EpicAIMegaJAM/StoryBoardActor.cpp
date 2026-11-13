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
	constexpr float SlotSpacing  = 130.f;
	constexpr float SlotX        = 2.f;
	constexpr float SlotZ        = 10.f;
	constexpr float NoteX        = 6.f;
	constexpr float TrayZ        = -60.f;
	constexpr float NoteSpacing  = 70.f;
	constexpr float DragX        = 14.f;
	constexpr float SnapDistance = 55.f;
	constexpr float BlockZ       = 80.f;   // confirmed blocks row height
	constexpr float BlockSpacing = 150.f;

	FString BoardShortName(const FGameplayTag& Tag)
	{
		const FString Full = Tag.ToString();
		int32 Idx = INDEX_NONE;
		return Full.FindLastChar('.', Idx) ? Full.Mid(Idx + 1) : Full;
	}

	FVector SlotLocal(int32 Index, int32 Count)
	{
		return FVector(SlotX, (Index - (Count-1)*0.5f) * SlotSpacing, SlotZ);
	}

	FVector TrayLocal(int32 Index, int32 Count)
	{
		return FVector(NoteX, (Index - (Count-1)*0.5f) * NoteSpacing, TrayZ);
	}

	FVector BlockLocal(int32 Index, int32 Count)
	{
		return FVector(SlotX, (Index - (Count-1)*0.5f) * BlockSpacing, BlockZ);
	}

	void Tint(UStaticMeshComponent* Mesh, UMaterialInterface* Mat, const FLinearColor& Color)
	{
		if (!Mesh || !Mat) return;
		if (UMaterialInstanceDynamic* MID = Mesh->CreateDynamicMaterialInstance(0, Mat))
			MID->SetVectorParameterValue(TEXT("Color"), Color);
	}

	void StyleText(UTextRenderComponent* T, float Size, FColor Color)
	{
		T->SetHorizontalAlignment(EHTA_Center);
		T->SetVerticalAlignment(EVRTA_TextCenter);
		T->SetWorldSize(Size);
		T->SetTextRenderColor(Color);
	}
}

AStoryBoardActor::AStoryBoardActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface>
		MatFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (CubeFinder.Succeeded()) CubeMesh = CubeFinder.Object;
	if (MatFinder.Succeeded()) BasicMat = MatFinder.Object;

	BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
	BoardMesh->SetupAttachment(SceneRoot);
	BoardMesh->SetStaticMesh(CubeMesh);
	BoardMesh->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	BoardMesh->SetRelativeScale3D(FVector(0.02f, 6.f, 2.8f));
	BoardMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	BoardCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BoardCamera"));
	BoardCamera->SetupAttachment(SceneRoot);
	BoardCamera->SetRelativeLocation(FVector(360.f, 0.f, 10.f));
	BoardCamera->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));

	TitleText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TitleText"));
	TitleText->SetupAttachment(SceneRoot);
	TitleText->SetRelativeLocation(FVector(3.f, 0.f, 115.f));
	StyleText(TitleText, 12.f, FColor(235, 225, 205));
	TitleText->SetText(FText::GetEmpty());

	StatusText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusText"));
	StatusText->SetupAttachment(SceneRoot);
	StatusText->SetRelativeLocation(FVector(3.f, 0.f, -20.f));
	StyleText(StatusText, 9.f, FColor(220, 70, 55));
	StatusText->SetText(FText::GetEmpty());

	HintText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HintText"));
	HintText->SetupAttachment(SceneRoot);
	HintText->SetRelativeLocation(FVector(3.f, 0.f, -100.f));
	StyleText(HintText, 5.f, FColor(160, 150, 130));
	HintText->SetText(FText::FromString(
		TEXT("Drag notes   |   Enter: Rewrite   |   Backspace: Leave")));
	HintText->SetHiddenInGame(true);
}

void AStoryBoardActor::BeginPlay()
{
	Super::BeginPlay();
	BuildSlots();
	
	if (UStoryStateSubsystem* Story = GetStory())
	{
		if (Board)
		{
			for (const int32 Index : Story->GetConfirmedStorylines(Board))
			{
				AddConfirmedBlock(Index);
			}
		}
	}
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
	BoardWidth = FMath::Max(Count * SlotSpacing + 80.f, 700.f);

	BoardMesh->SetRelativeScale3D(FVector(0.02f, BoardWidth / 100.f, 2.8f));
	BoardCamera->SetRelativeLocation(FVector(BoardWidth * 0.5f + 60.f, 0.f, 10.f));
	Tint(BoardMesh, BasicMat, FLinearColor(0.06f, 0.055f, 0.05f));
	TitleText->SetText(Board->SceneTitle);

	for (int32 i = 0; i < Count; ++i)
	{
		FBoardSlotVisual V;

		V.Mesh = NewObject<UStaticMeshComponent>(this);
		V.Mesh->SetStaticMesh(CubeMesh);
		V.Mesh->SetupAttachment(SceneRoot);
		V.Mesh->RegisterComponent();
		V.Mesh->SetRelativeLocation(SlotLocal(i, Count));
		V.Mesh->SetRelativeScale3D(FVector(0.01f, 0.95f, 0.5f));
		V.Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Tint(V.Mesh, BasicMat, FLinearColor(0.15f, 0.14f, 0.13f));

		V.Text = NewObject<UTextRenderComponent>(this);
		V.Text->SetupAttachment(SceneRoot);
		V.Text->RegisterComponent();
		V.Text->SetRelativeLocation(SlotLocal(i, Count) + FVector(1.f, 0.f, 28.f));
		StyleText(V.Text, 5.f, FColor(200, 190, 170));
		V.Text->SetText(Board->Slots[i].Label);

		SlotVisuals.Add(V);
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
		StyleText(Note.Text, 6.f, FColor(30, 25, 15));

		const FText Title = Story->GetClueTitle(Clue);
		Note.Text->SetText(Title.IsEmpty()
			? FText::FromString(BoardShortName(Clue)) : Title);

		Notes.Add(Note);
	}
}

void AStoryBoardActor::DestroyNotes()
{
	for (FBoardNote& N : Notes)
	{
		if (N.Text)  N.Text->DestroyComponent();
		if (N.Mesh)  N.Mesh->DestroyComponent();
		if (N.Root)  N.Root->DestroyComponent();
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
			Notes[n].Root->SetRelativeLocation(
				SlotLocal(SlotIndex, SlotCount) + FVector(NoteX - SlotX, 0.f, 0.f));
		}
		else
		{
			Unplaced.Add(n);
		}
	}

	for (int32 k = 0; k < Unplaced.Num(); ++k)
	{
		Notes[Unplaced[k]].Root->SetRelativeLocation(
			TrayLocal(k, Unplaced.Num()));
	}
}

void AStoryBoardActor::RefreshPins()
{
	UStoryStateSubsystem* Story = GetStory();
	const int32 Active = (Story && Board) ? Story->GetActiveStoryline(Board) : INDEX_NONE;

	for (FConfirmedBlock& Block : ConfirmedBlocks)
	{
		if (!Block.Pin) continue;
		Tint(Block.Pin, BasicMat,
			Block.StorylineIndex == Active
			? FLinearColor(1.f, 0.85f, 0.f)    // gold = active
			: FLinearColor(0.3f, 0.3f, 0.3f)); // grey = inactive
	}
}

void AStoryBoardActor::AddConfirmedBlock(int32 StorylineIndex)
{
	if (!Board || !Board->Storylines.IsValidIndex(StorylineIndex)) return;

	// Don't add duplicates
	for (const FConfirmedBlock& B : ConfirmedBlocks)
		if (B.StorylineIndex == StorylineIndex) return;

	const int32 Count = ConfirmedBlocks.Num() + 1;
	const int32 MyIndex = Count - 1;

	FConfirmedBlock Block;
	Block.StorylineIndex = StorylineIndex;

	Block.Mesh = NewObject<UStaticMeshComponent>(this);
	Block.Mesh->SetStaticMesh(CubeMesh);
	Block.Mesh->SetupAttachment(SceneRoot);
	Block.Mesh->RegisterComponent();
	Block.Mesh->SetRelativeLocation(BlockLocal(MyIndex, Count));
	Block.Mesh->SetRelativeScale3D(FVector(0.012f, 1.1f, 0.42f));
	Block.Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Block.Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Block.Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Tint(Block.Mesh, BasicMat, FLinearColor(0.12f, 0.11f, 0.18f));

	Block.Text = NewObject<UTextRenderComponent>(this);
	Block.Text->SetupAttachment(SceneRoot);
	Block.Text->RegisterComponent();
	Block.Text->SetRelativeLocation(BlockLocal(MyIndex, Count) + FVector(2.f, 0.f, 0.f));
	StyleText(Block.Text, 6.f, FColor(220, 210, 240));
	Block.Text->SetText(Board->Storylines[StorylineIndex].Title);

	// Pin dot in the top-right corner of the block
	Block.Pin = NewObject<UStaticMeshComponent>(this);
	Block.Pin->SetStaticMesh(CubeMesh);
	Block.Pin->SetupAttachment(SceneRoot);
	Block.Pin->RegisterComponent();
	Block.Pin->SetRelativeLocation(
		BlockLocal(MyIndex, Count) + FVector(3.f, 48.f, 20.f));
	Block.Pin->SetRelativeScale3D(FVector(0.02f, 0.08f, 0.08f));
	Block.Pin->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Tint(Block.Pin, BasicMat, FLinearColor(0.3f, 0.3f, 0.3f));

	ConfirmedBlocks.Add(Block);

	// Reposition all blocks so they stay centered as new ones are added
	const int32 Total = ConfirmedBlocks.Num();
	for (int32 i = 0; i < Total; ++i)
	{
		const FVector NewLoc = BlockLocal(i, Total);
		ConfirmedBlocks[i].Mesh->SetRelativeLocation(NewLoc);
		ConfirmedBlocks[i].Text->SetRelativeLocation(NewLoc + FVector(2.f, 0.f, 0.f));
		ConfirmedBlocks[i].Pin->SetRelativeLocation(NewLoc + FVector(3.f, 48.f, 20.f));
	}

	RefreshPins();
}

void AStoryBoardActor::HandleBlockClick(int32 StorylineIndex)
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Story || !Board) return;

	// Clicking the active block sets reality to None
	const int32 NewActive = (Story->GetActiveStoryline(Board) == StorylineIndex) ? INDEX_NONE : StorylineIndex;
	Story->SetActiveStoryline(Board, NewActive);
	RefreshPins();

	StatusText->SetTextRenderColor(FColor(120, 200, 120));
	StatusText->SetText(NewActive == INDEX_NONE
		? FText::FromString(TEXT("Reality cleared."))
		: Board->Storylines[NewActive].Title);
}

void AStoryBoardActor::ActivateStoryline(int32 Index)
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Story || !Board) return;

	// Undo current active facts
	if (Board->Storylines.IsValidIndex(ActiveStoryline))
	{
		for (const FGameplayTag& Fact : Board->Storylines[ActiveStoryline].ResultFacts)
			Story->SetFact(Fact, false);
	}

	ActiveStoryline = Index;

	// Apply new active facts
	if (Board->Storylines.IsValidIndex(ActiveStoryline))
	{
		for (const FGameplayTag& Fact : Board->Storylines[ActiveStoryline].ResultFacts)
			Story->SetFact(Fact, true);
	}
}

int32 AStoryBoardActor::GetActiveStoryline() const
{
	return ActiveStoryline;
}

FText AStoryBoardActor::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return bActive ? FText::GetEmpty() : InteractionText;
}

void AStoryBoardActor::Interact_Implementation(AActor* Interactor)
{
	if (bActive || !Board) return;

	APawn* Pawn = Cast<APawn>(Interactor);
	APlayerController* PC = Pawn
		? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (PC) EnterBoard(PC);
}

void AStoryBoardActor::EnterBoard(APlayerController* PC)
{
	if (!PC) return;

	ActivePC = PC;
	bActive = true;

	if (APawn* Pawn = PC->GetPawn())
	{
		HiddenPawn = Pawn;
	}

	// Only start the fade. Everything else waits until black.
	StartScreenFadeOut();

	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);

	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(Mode);
	PC->SetShowMouseCursor(true);

	bPrevLMB   = PC->IsInputKeyDown(EKeys::LeftMouseButton);
	bPrevEnter = PC->IsInputKeyDown(EKeys::Enter);
	bPrevBack  = PC->IsInputKeyDown(EKeys::BackSpace);

	InputReadyTime = GetWorld()->GetTimeSeconds() + 0.7f; // cover the whole sequence

	SetActorTickEnabled(true);
}

void AStoryBoardActor::ExitBoard()
{
	if (!bActive || !ActivePC) return;

	bActive = false;
	DraggedIndex = INDEX_NONE;

	// Start fade to black – the rest happens inside OnFadeOutFinishedForExit
	StartScreenFadeOutForExit();
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

	const bool bLMB   = ActivePC->IsInputKeyDown(EKeys::LeftMouseButton);
	const bool bEnter = ActivePC->IsInputKeyDown(EKeys::Enter);
	const bool bBack  = ActivePC->IsInputKeyDown(EKeys::BackSpace);

	const bool bLMBPressed    = bLMB   && !bPrevLMB;
	const bool bLMBReleased   = !bLMB  && bPrevLMB;
	const bool bEnterPressed  = bEnter && !bPrevEnter;
	const bool bBackPressed   = bBack  && !bPrevBack;

	bPrevLMB   = bLMB;
	bPrevEnter = bEnter;
	bPrevBack  = bBack;

	if (GetWorld()->GetTimeSeconds() < InputReadyTime) return;

	if (bBackPressed)  { ExitBoard(); return; }
	if (bEnterPressed) { TryRewrite(); if (!bActive) return; }
	if (bLMBPressed)   BeginDrag();
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

	// Check confirmed blocks first
	for (const FConfirmedBlock& Block : ConfirmedBlocks)
	{
		if (Block.Mesh == Clicked)
		{
			HandleBlockClick(Block.StorylineIndex);
			return;
		}
	}

	// Then check notes
	for (int32 i = 0; i < Notes.Num(); ++i)
	{
		if (Notes[i].Mesh == Clicked)
		{
			DraggedIndex = i;
			StatusText->SetText(FText::GetEmpty());
			for (const FStoryBoardSlot& BoardSlot : Board->Slots)
			{
				if (Story->GetPlacedClue(BoardSlot.SlotTag) == Notes[i].Clue)
					Story->ClearSlot(BoardSlot.SlotTag);
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

	const FTransform T = GetActorTransform();
	const FVector PlanePoint  = T.TransformPosition(FVector(DragX, 0.f, 0.f));
	const FVector PlaneNormal = GetActorForwardVector();
	const FVector HitPoint    = FMath::LinePlaneIntersection(
		Origin, Origin + Direction * 20000.f, PlanePoint, PlaneNormal);

	FVector Local = T.InverseTransformPosition(HitPoint);
	Local.X = DragX;
	Local.Y = FMath::Clamp(Local.Y, -BoardWidth * 0.5f + 30.f, BoardWidth * 0.5f - 30.f);
	Local.Z = FMath::Clamp(Local.Z, -85.f, BlockZ - 10.f);

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
		double BestDist = SnapDistance;
		for (int32 i = 0; i < Count; ++i)
		{
			const FVector SlotPos = SlotLocal(i, Count);
			const double Dist = FVector2D::Distance(
				FVector2D(Local.Y, Local.Z), FVector2D(SlotPos.Y, SlotPos.Z));
			if (Dist < BestDist) { BestDist = Dist; Best = i; }
		}
		if (Best != INDEX_NONE)
			Story->PlaceClue(Board->Slots[Best].SlotTag, Notes[DraggedIndex].Clue);
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

	AddConfirmedBlock(Index);
	RefreshPins();

	StatusText->SetTextRenderColor(FColor(120, 200, 120));
	StatusText->SetText(Board->Storylines[Index].Title);
	ExitBoard();
}
// ============================================================
// Screen Fade System (correct order)
// ============================================================

void AStoryBoardActor::StartScreenFadeOut()
{
	if (!ActivePC || !ActivePC->PlayerCameraManager) return;

	// Make sure no previous fade is stuck
	ActivePC->PlayerCameraManager->StopCameraFade();

	ActivePC->PlayerCameraManager->StartCameraFade(
		0.f, 1.f, 0.20f, FLinearColor::Black, false, true);

	GetWorldTimerManager().ClearTimer(FadeTimer);
	GetWorldTimerManager().SetTimer(
		FadeTimer, this, &AStoryBoardActor::OnFadeOutFinished, 0.21f, false);
}

void AStoryBoardActor::OnFadeOutFinished()
{
	if (!ActivePC) return;

	// === Everything happens while the screen is black ===

	// 1. Hide player
	if (APawn* Pawn = HiddenPawn.Get())
	{
		Pawn->SetActorHiddenInGame(true);
	}

	// 2. Instant camera cut
	ActivePC->SetViewTarget(this);

	// 3. Build the board content
	BuildNotes();
	RefreshLayout();
	RefreshPins();
	StatusText->SetText(FText::GetEmpty());
	HintText->SetHiddenInGame(false);

	// 4. Short hold in black, then fade in
	GetWorldTimerManager().SetTimer(
		FadeTimer, this, &AStoryBoardActor::StartScreenFadeIn, 0.12f, false);
}

void AStoryBoardActor::StartScreenFadeIn()
{
	if (!ActivePC || !ActivePC->PlayerCameraManager) return;

	ActivePC->PlayerCameraManager->StopCameraFade(); // safety
	ActivePC->PlayerCameraManager->StartCameraFade(
		1.f, 0.f, 0.28f, FLinearColor::Black, false, false);
}

void AStoryBoardActor::StartScreenFadeOutForExit()
{
	if (!ActivePC || !ActivePC->PlayerCameraManager) return;

	ActivePC->PlayerCameraManager->StopCameraFade();

	ActivePC->PlayerCameraManager->StartCameraFade(
		0.f, 1.f, 0.20f, FLinearColor::Black, false, true);

	GetWorldTimerManager().ClearTimer(FadeTimer);
	GetWorldTimerManager().SetTimer(
		FadeTimer, this, &AStoryBoardActor::OnFadeOutFinishedForExit, 0.21f, false);
}

void AStoryBoardActor::OnFadeOutFinishedForExit()
{
	// Keep the pointer before FinishExit clears it
	APlayerController* PC = ActivePC;

	// Restore player + camera while black
	if (APawn* Pawn = HiddenPawn.Get())
	{
		Pawn->SetActorHiddenInGame(false);
	}
	HiddenPawn.Reset();

	if (PC && PC->GetPawn())
	{
		PC->SetViewTarget(PC->GetPawn());
	}

	// Normal exit cleanup
	FinishExit();

	// Fade back in
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StopCameraFade();
		PC->PlayerCameraManager->StartCameraFade(
			1.f, 0.f, 0.28f, FLinearColor::Black, false, false);
	}
}