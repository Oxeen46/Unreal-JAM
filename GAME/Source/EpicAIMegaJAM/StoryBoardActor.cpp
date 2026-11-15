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
#include "GameSettingsSubsystem.h"
#include "Components/WidgetComponent.h"
#include "NoteCardWidget.h"
#include "Engine/Texture2D.h"

namespace
{
	// Board-local space: X = out of the board, Y = sideways (screen-left is +Y), Z = up.
	// Each storyboard is a cluster laid out around its own origin.
	constexpr float SlotSpacing = 130.f;
	constexpr float SlotX = 2.f;
	constexpr float NoteX = 6.f;
	constexpr float NoteSpacing = 70.f;
	constexpr float DragX = 14.f;
	constexpr float SnapDistance = 60.f;
	constexpr float CellPadding = 160.f;
	constexpr float SlotZ = 40.f;
	constexpr float TrayZ = -55.f;
	constexpr float TrayRowSpacing = 85.f;
	constexpr float ViewZ = -8.f;
	constexpr float CellHeight = 480.f;
	constexpr float NoteWidth = 60.f;
	constexpr float NoteHeight = 70.f;
	constexpr int32 MaxGridColumns = 2;

	// Warm and dark, easy on the eyes
	const FLinearColor WallColor(0.075f, 0.048f, 0.03f);
	const FLinearColor PanelColor(0.11f, 0.07f, 0.045f);
	const FLinearColor PanelSelectedColor(0.16f, 0.105f, 0.065f);
	const FLinearColor SlotColor(0.045f, 0.03f, 0.02f);
	const FLinearColor NoteColor(0.85f, 0.76f, 0.5f);
	const FLinearColor PinColor(1.f, 0.85f, 0.f);
	const FColor TitleColor(245, 242, 235);
	const FColor Gold(255, 215, 0);
	const FColor Red(220, 70, 55);
	const FColor Green(120, 200, 120);

	FString BoardShortName(const FGameplayTag& Tag)
	{
		const FString Full = Tag.ToString();
		int32 Idx = INDEX_NONE;
		return Full.FindLastChar('.', Idx) ? Full.Mid(Idx + 1) : Full;
	}

	float PageWidth(int32 NoteCount)
	{
		return FMath::Max(NoteCount * SlotSpacing + 100.f, 520.f);
	}

	// Slot 1 is on the left of the screen, so slots run toward -Y. Relative to a cluster origin.
	FVector SlotLocal(int32 Index, int32 Count)
	{
		return FVector(SlotX, -(Index - (Count - 1) * 0.5f) * SlotSpacing, SlotZ);
	}

	UMaterialInstanceDynamic* Tint(UStaticMeshComponent* Mesh, UMaterialInterface* Mat, const FLinearColor& Color)
	{
		if (!Mesh || !Mat) return nullptr;
		UMaterialInstanceDynamic* MID = Mesh->CreateDynamicMaterialInstance(0, Mat);
		if (MID) MID->SetVectorParameterValue(TEXT("Color"), Color);
		return MID;
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

	// The wall everything is pinned to
	BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
	BoardMesh->SetupAttachment(SceneRoot);
	BoardMesh->SetStaticMesh(CubeMesh);
	BoardMesh->SetRelativeLocation(FVector(0.f, 0.f, ViewZ));
	BoardMesh->SetRelativeScale3D(FVector(0.02f, 8.f, 4.4f));
	BoardMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	ContentRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ContentRoot"));
	ContentRoot->SetupAttachment(SceneRoot);

	// Follows the selected storyboard. Holds the tray, status and hint.
	FocusRoot = CreateDefaultSubobject<USceneComponent>(TEXT("FocusRoot"));
	FocusRoot->SetupAttachment(ContentRoot);

	// In front of the board (+X), looking back at it
	BoardCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BoardCamera"));
	BoardCamera->SetupAttachment(SceneRoot);
	BoardCamera->SetRelativeLocation(FVector(340.f, 0.f, ViewZ));
	BoardCamera->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));

	StatusText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusText"));
	StatusText->SetupAttachment(FocusRoot);
	StatusText->SetRelativeLocation(FVector(3.f, 0.f, 5.f));
	StyleText(StatusText, 9.f, Red);
	StatusText->SetText(FText::GetEmpty());

	HintText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HintText"));
	HintText->SetupAttachment(FocusRoot);
	HintText->SetRelativeLocation(FVector(3.f, 0.f, -118.f));
	StyleText(HintText, 4.5f, FColor(170, 155, 135));
	HintText->SetText(FText::GetEmpty());
	HintText->SetHiddenInGame(true);
}

UStoryStateSubsystem* AStoryBoardActor::GetStory() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UStoryStateSubsystem>() : nullptr;
}

void AStoryBoardActor::BeginPlay()
{
	Super::BeginPlay();

	BoardMID = BoardMesh->CreateDynamicMaterialInstance(0, BasicMat);
	if (BoardMID) BoardMID->SetVectorParameterValue(TEXT("Color"), WallColor);

	LayoutGrid();

	if (UStoryStateSubsystem* Story = GetStory())
	{
		Story->OnClueCollected.AddDynamic(this, &AStoryBoardActor::HandleClueCollected);

		const TArray<int32> Revealed = RevealedList();
		const int32 Active = Story->GetActiveStoryline(Board);
		Selected = Revealed.Contains(Active) ? Active : (Revealed.Num() > 0 ? Revealed[0] : INDEX_NONE);
	}

	FocusLoc = (Selected != INDEX_NONE) ? ClusterOrigin(Selected) : FVector::ZeroVector;
	FocusRoot->SetRelativeLocation(FocusLoc);
	BuildBoard();
}

void AStoryBoardActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UStoryStateSubsystem* Story = GetStory())
	{
		Story->OnClueCollected.RemoveDynamic(this, &AStoryBoardActor::HandleClueCollected);
	}
	Super::EndPlay(EndPlayReason);
}

// ---------------- Layout ----------------

void AStoryBoardActor::LayoutGrid()
{
	const int32 Count = Board ? Board->Storylines.Num() : 0;

	float MaxWidth = 520.f;
	if (Board)
	{
		for (const FStoryline& Line : Board->Storylines)
		{
			MaxWidth = FMath::Max(MaxWidth, PageWidth(Line.Notes.Num()));
		}
	}

	CellWidth = MaxWidth + CellPadding;
	GridCols = FMath::Clamp(Count, 1, MaxGridColumns);
	GridRows = FMath::Max(1, FMath::DivideAndRoundUp(Count, GridCols));
	WallWidth = GridCols * CellWidth + 80.f;
	WallHeight = GridRows * CellHeight + 60.f;
	OverviewDist = FMath::Max(WallWidth * 0.5f, WallHeight / 1.125f) + 60.f;

	BoardMesh->SetRelativeLocation(FVector(0.f, 0.f, ViewZ));
	BoardMesh->SetRelativeScale3D(FVector(0.02f, WallWidth / 100.f, WallHeight / 100.f));
}

FVector AStoryBoardActor::ClusterOrigin(int32 StorylineIndex) const
{
	const int32 Col = StorylineIndex % GridCols;
	const int32 Row = StorylineIndex / GridCols;
	return FVector(0.f, ((GridCols - 1) * 0.5f - Col) * CellWidth, ((GridRows - 1) * 0.5f - Row) * CellHeight);
}

TArray<int32> AStoryBoardActor::RevealedList() const
{
	TArray<int32> Result;
	UStoryStateSubsystem* Story = GetStory();
	if (!Board || !Story) return Result;

	for (int32 i = 0; i < Board->Storylines.Num(); ++i)
	{
		if (Story->IsStorylineRevealed(Board, i))
		{
			Result.Add(i);
		}
	}
	return Result;
}

// ---------------- Building ----------------

UStaticMeshComponent* AStoryBoardActor::AddCube(USceneComponent* Parent, const FVector& Loc, const FVector& Scale, const FLinearColor& Color, bool bClickable)
{
	UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this);
	Mesh->SetStaticMesh(CubeMesh);
	Mesh->SetupAttachment(Parent);
	Mesh->RegisterComponent();
	Mesh->SetRelativeLocation(Loc);
	Mesh->SetRelativeScale3D(Scale);

	if (bClickable)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
	else
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (UMaterialInstanceDynamic* MID = Tint(Mesh, BasicMat, Color))
	{
		FDimMesh Entry;
		Entry.MID = MID;
		Entry.Base = Color;
		DimMeshes.Add(Entry);
	}

	BoardComponents.Add(Mesh);
	return Mesh;
}

UTextRenderComponent* AStoryBoardActor::AddText(USceneComponent* Parent, const FVector& Loc, const FText& Text, float Size, FColor Color)
{
	UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this);
	Label->SetupAttachment(Parent);
	Label->RegisterComponent();
	Label->SetRelativeLocation(Loc);
	StyleText(Label, Size, Color);
	Label->SetText(Text);

	FDimText Entry;
	Entry.Text = Label;
	Entry.Base = Color;
	DimTexts.Add(Entry);

	BoardComponents.Add(Label);
	return Label;
}

USceneComponent* AStoryBoardActor::AddNote(USceneComponent* Parent, const FVector& Loc, FGameplayTag Clue, int32 Order, bool bClickable)
{
	USceneComponent* Root = NewObject<USceneComponent>(this);
	Root->SetupAttachment(Parent);
	Root->RegisterComponent();
	Root->SetRelativeLocation(Loc);
	BoardComponents.Add(Root);

	// The paper
	UStaticMeshComponent* Mesh = AddCube(Root, FVector::ZeroVector, FVector(0.015f, NoteWidth / 100.f, NoteHeight / 100.f), NoteColor, bClickable);

	UStoryStateSubsystem* Story = GetStory();
	const FText Title = Story ? Story->GetClueTitle(Clue) : FText::GetEmpty();
	const FText Name = Title.IsEmpty() ? FText::FromString(BoardShortName(Clue)) : Title;
	UTexture2D* Icon = Story ? Story->GetClueIcon(Clue) : nullptr;

	if (NoteCardClass)
	{
		// The card: the icon with the name under it, drawn in front of the paper
		UWidgetComponent* Card = NewObject<UWidgetComponent>(this);
		Card->SetupAttachment(Root);
		Card->SetWidgetSpace(EWidgetSpace::World);
		Card->SetWidgetClass(NoteCardClass);
		Card->SetDrawSize(FVector2D(240.f, 280.f));
		Card->SetRelativeScale3D(FVector(1.f, 0.25f, 0.25f)); // 240 x 280 pixels = 60 x 70 units
		Card->SetRelativeLocation(FVector(1.f, 0.f, 0.f));
		Card->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Card->SetRedrawTime(0.25f);
		Card->RegisterComponent();
		Card->InitWidget();

		if (UNoteCardWidget* CardWidget = Cast<UNoteCardWidget>(Card->GetUserWidgetObject()))
		{
			CardWidget->SetNote(Icon, Name, Order);
		}

		BoardComponents.Add(Card);
		DimWidgets.Add(Card);
	}
	else
	{
		// No card widget set yet: fall back to plain text
		AddText(Root, FVector(1.f, 0.f, 0.f), FText::FromString(FString::Printf(TEXT("#%d\n%s"), Order, *Name.ToString())), 6.f, FColor(35, 28, 18));
	}

	if (bClickable)
	{
		FBoardNote Note;
		Note.Clue = Clue;
		Note.Root = Root;
		Note.Mesh = Mesh;
		Notes.Add(Note);
	}
	return Root;
}

bool AStoryBoardActor::IsRelevantClue(FGameplayTag Clue) const
{
	if (!Board) return false;
	if (Board->DecoyClues.Contains(Clue)) return true;
	for (const FStoryline& Line : Board->Storylines)
	{
		if (Line.IndexOfClue(Clue) != INDEX_NONE) return true;
	}
	return false;
}

// The number printed on a note: its position in its own storyboard.
// Decoys get a stable made-up number so they look like the rest.
int32 AStoryBoardActor::NoteOrder(FGameplayTag Clue) const
{
	int32 Longest = 1;
	if (Board)
	{
		for (const FStoryline& Line : Board->Storylines)
		{
			const int32 Position = Line.IndexOfClue(Clue);
			if (Position != INDEX_NONE) return Position + 1;
			Longest = FMath::Max(Longest, Line.Notes.Num());
		}
	}
	return static_cast<int32>(GetTypeHash(Clue) % static_cast<uint32>(Longest)) + 1;
}

void AStoryBoardActor::ClearBoard()
{
	for (UActorComponent* Comp : BoardComponents)
	{
		if (IsValid(Comp))
		{
			Comp->DestroyComponent();
		}
	}
	BoardComponents.Reset();
	Notes.Reset();
	Panels.Reset();
	DimMeshes.Reset();
	DimTexts.Reset();
	DimWidgets.Reset();
	DraggedIndex = INDEX_NONE;
}

void AStoryBoardActor::BuildBoard()
{
	ClearBoard();

	UStoryStateSubsystem* Story = GetStory();
	if (!Board || !Story) return;

	for (int32 i = 0; i < Board->Storylines.Num(); ++i)
	{
		if (Story->IsStorylineRevealed(Board, i))
		{
			BuildCluster(i);
		}
	}

	if (bActive && !bOverview)
	{
		BuildTray();
	}

	ApplyDim(CurrentDim);
}

// The authored description: the newest news about this storyboard
FText AStoryBoardActor::PickDescription(int32 StorylineIndex) const
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Board || !Story || !Board->Storylines.IsValidIndex(StorylineIndex)) return FText::GetEmpty();

	const FStoryline& Line = Board->Storylines[StorylineIndex];
	if (Story->IsStorylineUnlocked(Board, StorylineIndex)) return Line.CompletedDescription;

	const int32 Progress = Story->GetStorylineProgress(Board, StorylineIndex);
	const FGameplayTag LastFound = Story->GetLastCollectedClueFor(Board, StorylineIndex);
	const int32 FoundIndex = LastFound.IsValid() ? Line.IndexOfClue(LastFound) : INDEX_NONE;

	// A clue you found but haven't placed yet is the newest news
	if (FoundIndex != INDEX_NONE && FoundIndex >= Progress && !Line.Notes[FoundIndex].FoundDescription.IsEmpty())
	{
		return Line.Notes[FoundIndex].FoundDescription;
	}

	// Otherwise, the last note you placed
	if (Progress > 0 && Line.Notes.IsValidIndex(Progress - 1) && !Line.Notes[Progress - 1].PlacedDescription.IsEmpty())
	{
		return Line.Notes[Progress - 1].PlacedDescription;
	}

	return (FoundIndex != INDEX_NONE) ? Line.Notes[FoundIndex].FoundDescription : FText::GetEmpty();
}

void AStoryBoardActor::BuildCluster(int32 StorylineIndex)
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Board || !Story || !Board->Storylines.IsValidIndex(StorylineIndex)) return;

	const FStoryline& Line = Board->Storylines[StorylineIndex];
	const FVector O = ClusterOrigin(StorylineIndex);
	const int32 N = Line.Notes.Num();
	const int32 Progress = Story->GetStorylineProgress(Board, StorylineIndex);
	const bool bUnlocked = Story->IsStorylineUnlocked(Board, StorylineIndex);
	const bool bIsActive = Story->GetActiveStoryline(Board) == StorylineIndex;
	const float Width = PageWidth(N);

	// The backdrop card. Click it to zoom to this storyboard.
	UStaticMeshComponent* Panel = AddCube(ContentRoot, O + FVector(1.f, 0.f, -7.f), FVector(0.012f, Width / 100.f, 3.95f),
		StorylineIndex == Selected ? PanelSelectedColor : PanelColor, true);
	FBoardPanel PanelEntry;
	PanelEntry.Mesh = Panel;
	PanelEntry.StorylineIndex = StorylineIndex;
	Panels.Add(PanelEntry);

	// Big white title ("?" until the storyboard is complete)
	AddText(ContentRoot, O + FVector(3.f, 0.f, 140.f), bUnlocked ? Line.Title : FText::FromString(TEXT("?")), 20.f, TitleColor);

	if (bIsActive)
	{
		AddCube(ContentRoot, O + FVector(6.f, 0.f, 175.f), FVector(0.06f, 0.1f, 0.1f), PinColor, false);
	}

	// Your own description, based on the last found or placed clue
	const FText Description = PickDescription(StorylineIndex);
	if (!Description.IsEmpty())
	{
		AddText(ContentRoot, O + FVector(3.f, 0.f, 108.f), Description, 6.f, FColor(190, 175, 150));
	}

	if (bIsActive)
	{
		AddText(ContentRoot, O + FVector(3.f, 0.f, 92.f), FText::FromString(TEXT("ACTIVE REALITY")), 5.5f, Gold);
	}
	else if (bUnlocked && bActive)
	{
		AddText(ContentRoot, O + FVector(3.f, 0.f, 92.f), FText::FromString(TEXT("Press Enter to make this the reality")), 5.5f, FColor(150, 140, 120));
	}

	// The lineup: numbered slots, with the placed notes on them
	for (int32 i = 0; i < N; ++i)
	{
		const FVector Pos = O + SlotLocal(i, N);
		AddCube(ContentRoot, Pos, FVector(0.01f, 1.f, 0.84f), SlotColor, false);
		AddText(ContentRoot, Pos + FVector(1.f, 0.f, 0.f), FText::FromString(FString::FromInt(i + 1)), 14.f, FColor(95, 75, 60));

		if (i < Progress)
		{
			AddNote(ContentRoot, Pos + FVector(NoteX - SlotX, 0.f, 0.f), Line.Notes[i].Clue, i + 1, false);
		}
	}
}

// Every collected note that is not placed yet, under the selected storyboard
void AStoryBoardActor::BuildTray()
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Board || !Story || !Board->Storylines.IsValidIndex(Selected)) return;

	const float Width = PageWidth(Board->Storylines[Selected].Notes.Num());

	TArray<FGameplayTag> Tray;
	for (const FGameplayTag& Clue : Story->GetCollectedClues())
	{
		if (IsRelevantClue(Clue) && !Story->IsClueConsumed(Board, Clue))
		{
			Tray.Add(Clue);
		}
	}

	const int32 Columns = FMath::Max(1, FMath::FloorToInt((Width - 60.f) / NoteSpacing));
	for (int32 k = 0; k < Tray.Num(); ++k)
	{
		const int32 Row = k / Columns;
		const int32 InThisRow = FMath::Min(Columns, Tray.Num() - Row * Columns);
		const int32 Column = k % Columns;
		const FVector Pos(NoteX, (Column - (InThisRow - 1) * 0.5f) * NoteSpacing, TrayZ - Row * TrayRowSpacing);
		AddNote(FocusRoot, Pos, Tray[k], NoteOrder(Tray[k]), true);
	}
}

void AStoryBoardActor::ApplyDim(float Dim)
{
	if (BoardMID)
	{
		BoardMID->SetVectorParameterValue(TEXT("Color"), FLinearColor(WallColor.R * Dim, WallColor.G * Dim, WallColor.B * Dim, 1.f));
	}

	for (const FDimMesh& Entry : DimMeshes)
	{
		if (IsValid(Entry.MID))
		{
			Entry.MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(Entry.Base.R * Dim, Entry.Base.G * Dim, Entry.Base.B * Dim, 1.f));
		}
	}

	for (const FDimText& Entry : DimTexts)
	{
		if (IsValid(Entry.Text))
		{
			Entry.Text->SetTextRenderColor(FColor(
				static_cast<uint8>(Entry.Base.R * Dim), static_cast<uint8>(Entry.Base.G * Dim), static_cast<uint8>(Entry.Base.B * Dim), 255));
		}
	}
	for (UWidgetComponent* Card : DimWidgets)
	{
		if (IsValid(Card))
		{
			Card->SetTintColorAndOpacity(FLinearColor(Dim, Dim, Dim, 1.f));
		}
	}

	const bool bShowTexts = Dim > 0.5f;
	StatusText->SetVisibility(bShowTexts);
	HintText->SetVisibility(bShowTexts);
}

void AStoryBoardActor::SetStatus(const FText& Text, FColor Color)
{
	StatusText->SetTextRenderColor(Color);
	StatusText->SetText(Text);
}

void AStoryBoardActor::HandleClueCollected(FGameplayTag Clue)
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Board || !Story) return;

	// Is this the first clue we have for some storyboard? Then that storyboard just opened.
	for (const FStoryline& Line : Board->Storylines)
	{
		if (Line.bRevealedFromStart || Line.IndexOfClue(Clue) == INDEX_NONE) continue;

		int32 Found = 0;
		for (const FStoryNote& Note : Line.Notes)
		{
			if (Story->HasClue(Note.Clue)) ++Found;
		}
		if (Found == 1 && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("A new storyboard has opened."));
		}
	}

	if (!bActive)
	{
		BuildBoard(); // keep the resting display up to date
	}
}

// ---------------- Camera and selection ----------------

void AStoryBoardActor::UpdateCameraTargets()
{
	if (bOverview || !Board || !Board->Storylines.IsValidIndex(Selected))
	{
		TargetCamLoc = FVector(OverviewDist, 0.f, ViewZ);
		return;
	}

	const FVector O = ClusterOrigin(Selected);
	const float Width = PageWidth(Board->Storylines[Selected].Notes.Num());
	TargetCamLoc = FVector(FMath::Max(Width * 0.5f + 60.f, 340.f), O.Y, O.Z + ViewZ);
}

void AStoryBoardActor::UpdateHint()
{
	if (!Board) return;

	FString Text;
	if (bOverview)
	{
		Text = TEXT("Whole board   |   Click a storyboard, or Left/Right   |   Z or scroll up: zoom in   |   Backspace: leave");
	}
	else
	{
		const TArray<int32> Revealed = RevealedList();
		Text = FString::Printf(TEXT("Storyboard %d / %d   |   Drag notes in order   |   Left/Right: next   |   Enter: make reality   |   Z: whole board   |   Backspace: leave"),
			FMath::Max(1, Revealed.IndexOfByKey(Selected) + 1), Revealed.Num());
	}
	HintText->SetText(FText::FromString(Text));
}

void AStoryBoardActor::SelectStoryline(int32 Index, bool bZoomIn)
{
	Selected = Index;
	if (bZoomIn) bOverview = false;

	SetStatus(FText::GetEmpty(), Red);
	UpdateCameraTargets();
	BuildBoard();
	UpdateHint();
}

void AStoryBoardActor::StepSelection(int32 Direction)
{
	const TArray<int32> Revealed = RevealedList();
	if (Revealed.Num() < 2) return;

	const int32 Position = FMath::Max(0, Revealed.IndexOfByKey(Selected));
	SelectStoryline(Revealed[(Position + Direction + Revealed.Num()) % Revealed.Num()], false);
}

void AStoryBoardActor::SetOverview(bool bNewOverview)
{
	if (bOverview == bNewOverview) return;
	bOverview = bNewOverview;

	UpdateCameraTargets();
	BuildBoard(); // the tray only exists while zoomed in
	UpdateHint();
}

// ---------------- Entering and leaving ----------------

FText AStoryBoardActor::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	if (bActive || RevealedList().Num() == 0) return FText::GetEmpty();
	return InteractionText;
}

void AStoryBoardActor::Interact_Implementation(AActor* Interactor)
{
	if (bActive || ActivePC || !Board) return;

	if (RevealedList().Num() == 0)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::White, TEXT("You have no leads yet."));
		}
		return;
	}

	const APawn* Pawn = Cast<APawn>(Interactor);
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (PC)
	{
		EnterBoard(PC);
	}
}

void AStoryBoardActor::EnterBoard(APlayerController* PC)
{
	UStoryStateSubsystem* Story = GetStory();
	const TArray<int32> Revealed = RevealedList();
	if (!Story || Revealed.Num() == 0) return;

	ActivePC = PC;
	bActive = true;
	bFlipping = false;
	bOverview = false;
	CurrentDim = 1.f;

	const int32 Active = Story->GetActiveStoryline(Board);
	Selected = Revealed.Contains(Active) ? Active : Revealed[0];

	FocusLoc = ClusterOrigin(Selected);
	FocusRoot->SetRelativeLocation(FocusLoc);

	// Start a little further back, then zoom in on the storyboard
	UpdateCameraTargets();
	CamLoc = TargetCamLoc;
	CamLoc.X *= 1.3f;
	BoardCamera->SetRelativeLocation(CamLoc);

	BuildBoard();
	UpdateHint();
	SetStatus(FText::GetEmpty(), Red);
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
	bPrevLeft = PC->IsInputKeyDown(EKeys::Left) || PC->IsInputKeyDown(EKeys::A);
	bPrevRight = PC->IsInputKeyDown(EKeys::Right) || PC->IsInputKeyDown(EKeys::D);
	bPrevZ = PC->IsInputKeyDown(EKeys::Z);
	InputReadyTime = GetWorld()->GetTimeSeconds() + BlendTime;

	SetActorTickEnabled(true);
}

void AStoryBoardActor::ExitBoard()
{
	if (!bActive || !ActivePC) return;

	bActive = false;
	bFlipping = false;
	bOverview = false;
	CurrentDim = 1.f;
	HintText->SetHiddenInGame(true);
	SetStatus(FText::GetEmpty(), Red);

	BuildBoard(); // back to the resting display, without the tray

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
	SetActorTickEnabled(false);
}

// ---------------- Reality change ----------------

void AStoryBoardActor::StartFlip()
{
	bFlipping = true;
	bFlipSwapped = false;
	FlipElapsed = 0.f;
	OnFlipStarted();
}

void AStoryBoardActor::TickFlip(float DeltaTime)
{
	FlipElapsed += DeltaTime;
	const float T = FMath::Clamp(FlipElapsed / FMath::Max(FlipDuration, 0.1f), 0.f, 1.f);

	// At the darkest point the board refreshes with the new reality
	if (!bFlipSwapped && T >= 0.5f)
	{
		bFlipSwapped = true;
		BuildBoard();
	}

	float Intensity = 1.f;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (const UGameSettingsSubsystem* Settings = GI->GetSubsystem<UGameSettingsSubsystem>())
		{
			Intensity = Settings->GetRealityEffect();
		}
	}

	const float Dip = FMath::Square(FMath::Sin(PI * T));
	const float Flutter = 1.f + FlutterAmount * Intensity * FMath::Sin(2.f * PI * 3.f * T);
	CurrentDim = FMath::Clamp(1.f - FlipDepth * Intensity * Dip * Flutter, 0.f, 1.f);
	ApplyDim(CurrentDim);

	if (T >= 1.f)
	{
		bFlipping = false;
		CurrentDim = 1.f;
		ApplyDim(1.f);
		OnFlipFinished();
	}
}

void AStoryBoardActor::ToggleActive()
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Story || !Board || !Board->Storylines.IsValidIndex(Selected)) return;

	if (!Story->IsStorylineUnlocked(Board, Selected))
	{
		SetStatus(FText::FromString(TEXT("This storyboard isn't complete yet.")), Red);
		return;
	}

	if (Story->GetActiveStoryline(Board) == Selected)
	{
		Story->SetActiveStoryline(Board, INDEX_NONE);
		SetStatus(FText::FromString(TEXT("Reality cleared.")), Green);
	}
	else
	{
		Story->SetActiveStoryline(Board, Selected);
		SetStatus(FText::FromString(FString::Printf(TEXT("Reality: %s"), *Board->Storylines[Selected].Title.ToString())), Green);
	}

	StartFlip();
}

// ---------------- Input ----------------

void AStoryBoardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!ActivePC) return;

	if (bActive)
	{
		// The camera glides to the selected storyboard, and the tray follows it
		CamLoc = FMath::VInterpTo(CamLoc, TargetCamLoc, DeltaTime, ZoomSpeed);
		BoardCamera->SetRelativeLocation(CamLoc);

		if (Selected != INDEX_NONE)
		{
			FocusLoc = FMath::VInterpTo(FocusLoc, ClusterOrigin(Selected), DeltaTime, ZoomSpeed);
			FocusRoot->SetRelativeLocation(FocusLoc);
		}
	}

	if (bFlipping)
	{
		TickFlip(DeltaTime);
	}

	if (!bActive) return;

	const bool bLMB = ActivePC->IsInputKeyDown(EKeys::LeftMouseButton);
	const bool bEnter = ActivePC->IsInputKeyDown(EKeys::Enter);
	const bool bBack = ActivePC->IsInputKeyDown(EKeys::BackSpace);
	const bool bLeft = ActivePC->IsInputKeyDown(EKeys::Left) || ActivePC->IsInputKeyDown(EKeys::A);
	const bool bRight = ActivePC->IsInputKeyDown(EKeys::Right) || ActivePC->IsInputKeyDown(EKeys::D);
	const bool bZ = ActivePC->IsInputKeyDown(EKeys::Z);

	const bool bLMBPressed = bLMB && !bPrevLMB;
	const bool bLMBReleased = !bLMB && bPrevLMB;
	const bool bEnterPressed = bEnter && !bPrevEnter;
	const bool bBackPressed = bBack && !bPrevBack;
	const bool bLeftPressed = bLeft && !bPrevLeft;
	const bool bRightPressed = bRight && !bPrevRight;
	const bool bZPressed = bZ && !bPrevZ;

	bPrevLMB = bLMB;
	bPrevEnter = bEnter;
	bPrevBack = bBack;
	bPrevLeft = bLeft;
	bPrevRight = bRight;
	bPrevZ = bZ;

	// Ignore input while the camera blends in or a reality change plays
	if (GetWorld()->GetTimeSeconds() < InputReadyTime || bFlipping) return;

	if (bBackPressed) { ExitBoard(); return; }
	if (bLeftPressed) { StepSelection(-1); return; }
	if (bRightPressed) { StepSelection(+1); return; }
	if (bEnterPressed) { ToggleActive(); return; }

	if (bZPressed || ActivePC->WasInputKeyJustPressed(EKeys::MouseScrollDown) || ActivePC->WasInputKeyJustPressed(EKeys::MouseScrollUp))
	{
		const bool bWheelDown = ActivePC->WasInputKeyJustPressed(EKeys::MouseScrollDown);
		const bool bWheelUp = ActivePC->WasInputKeyJustPressed(EKeys::MouseScrollUp);
		if (bZPressed) SetOverview(!bOverview);
		else if (bWheelDown) SetOverview(true);
		else if (bWheelUp) SetOverview(false);
		return;
	}

	if (bLMBPressed) BeginDrag();
	if (bLMB && DraggedIndex != INDEX_NONE) UpdateDrag();
	if (bLMBReleased && DraggedIndex != INDEX_NONE) EndDrag();
}

void AStoryBoardActor::BeginDrag()
{
	FHitResult Hit;
	ActivePC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	UPrimitiveComponent* Clicked = Hit.GetComponent();
	if (!Clicked) return;

	// Notes first (they sit in front of the cards)
	if (!bOverview)
	{
		for (int32 i = 0; i < Notes.Num(); ++i)
		{
			if (Notes[i].Mesh.Get() == Clicked)
			{
				DraggedIndex = i;
				SetStatus(FText::GetEmpty(), Red);
				return;
			}
		}
	}

	// Then a storyboard card: zoom to it
	for (const FBoardPanel& Panel : Panels)
	{
		if (Panel.Mesh.Get() == Clicked)
		{
			if (bOverview || Panel.StorylineIndex != Selected)
			{
				SelectStoryline(Panel.StorylineIndex, true);
			}
			return;
		}
	}
}

void AStoryBoardActor::UpdateDrag()
{
	if (!Notes.IsValidIndex(DraggedIndex) || !Board || !Board->Storylines.IsValidIndex(Selected)) return;

	FVector Origin, Direction;
	if (!ActivePC->DeprojectMousePositionToWorld(Origin, Direction)) return;

	const FTransform BoardTransform = GetActorTransform();
	const FVector PlanePoint = BoardTransform.TransformPosition(FVector(DragX, 0.f, 0.f));
	const FVector HitPoint = FMath::LinePlaneIntersection(Origin, Origin + Direction * 20000.f, PlanePoint, GetActorForwardVector());

	const FVector O = ClusterOrigin(Selected);
	const float HalfWidth = PageWidth(Board->Storylines[Selected].Notes.Num()) * 0.5f - 30.f;

	FVector Local = BoardTransform.InverseTransformPosition(HitPoint);
	Local.X = DragX;
	Local.Y = FMath::Clamp(Local.Y, O.Y - HalfWidth, O.Y + HalfWidth);
	Local.Z = FMath::Clamp(Local.Z, O.Z - 125.f, O.Z + 150.f);

	Notes[DraggedIndex].Root->SetWorldLocation(BoardTransform.TransformPosition(Local));
}

void AStoryBoardActor::EndDrag()
{
	UStoryStateSubsystem* Story = GetStory();
	if (!Story || !Board || !Notes.IsValidIndex(DraggedIndex) || !Board->Storylines.IsValidIndex(Selected))
	{
		DraggedIndex = INDEX_NONE;
		return;
	}

	const FVector Local = GetActorTransform().InverseTransformPosition(Notes[DraggedIndex].Root->GetComponentLocation());
	const FGameplayTag Clue = Notes[DraggedIndex].Clue;
	DraggedIndex = INDEX_NONE;

	// Which slot was it dropped on?
	const FVector O = ClusterOrigin(Selected);
	const int32 Count = Board->Storylines[Selected].Notes.Num();
	int32 Best = INDEX_NONE;
	double BestDistance = SnapDistance;
	for (int32 i = 0; i < Count; ++i)
	{
		const FVector SlotPos = O + SlotLocal(i, Count);
		const double Distance = FVector2D::Distance(FVector2D(Local.Y, Local.Z), FVector2D(SlotPos.Y, SlotPos.Z));
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			Best = i;
		}
	}

	if (Best != INDEX_NONE)
	{
		const bool bWasUnlocked = Story->IsStorylineUnlocked(Board, Selected);

		if (Story->TryPlaceNote(Board, Selected, Best, Clue))
		{
			if (!bWasUnlocked && Story->IsStorylineUnlocked(Board, Selected))
			{
				SetStatus(FText::FromString(FString::Printf(TEXT("Storyboard complete: %s"), *Board->Storylines[Selected].Title.ToString())), Green);
				StartFlip(); // the title changes from "?" to its name as the reality shifts
				return;
			}
			BuildBoard();
			return;
		}

		SetStatus(FText::FromString(TEXT("That doesn't fit.")), Red);
	}

	BuildBoard(); // the note goes back to the tray
}