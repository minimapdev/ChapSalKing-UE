#include "AI/CSKBotDebugDrawSubsystem.h"
#include "AI/CSKBotAIController.h"
#include "Framework/CSKGameInstance.h"

#include "Debug/DebugDrawService.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
#include "EngineUtils.h"

namespace
{
	constexpr float PanelPadding = 6.0f;
	constexpr float HeadOffsetZ = 130.0f;
	constexpr float MaxDrawDistance = 6000.0f;

	static const TCHAR* ObjectiveNames[CSKBotObjectiveCount] =
		{ TEXT("Wander"), TEXT("Flee"), TEXT("Jelly"), TEXT("Fight"), TEXT("Farm"), TEXT("Pestle") };
	static const FLinearColor ObjectiveColors[CSKBotObjectiveCount] =
	{
		FLinearColor(0.75f, 0.75f, 0.75f),   // Wander
		FLinearColor(1.0f, 0.35f, 0.35f),    // Flee
		FLinearColor(0.4f, 1.0f, 0.4f),      // Jelly
		FLinearColor(1.0f, 0.65f, 0.2f),     // Fight
		FLinearColor(0.3f, 0.9f, 1.0f),      // Farm
		FLinearColor(1.0f, 0.4f, 1.0f)       // Pestle
	};
}

void UCSKBotDebugDrawSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DrawHandle = UDebugDrawService::Register(
		TEXT("Game"),
		FDebugDrawDelegate::CreateUObject(this, &UCSKBotDebugDrawSubsystem::Draw));
}

void UCSKBotDebugDrawSubsystem::Deinitialize()
{
	UDebugDrawService::Unregister(DrawHandle);

	Super::Deinitialize();
}

void UCSKBotDebugDrawSubsystem::Draw(UCanvas* Canvas, APlayerController* PlayerController)
{
	UWorld* World = GetWorld();
	if (!World || !Canvas || !UCSKGameInstance::IsDrawDebugEnabled(this))
	{
		return;
	}

	const UFont* Font = GEngine->GetSmallFont();
	const FVector ViewLocation = Canvas->SceneView ? Canvas->SceneView->ViewLocation : FVector::ZeroVector;

	for (TActorIterator<ACSKBotAIController> It(World); It; ++It)
	{
		const ACSKBotAIController* Controller = *It;
		const FCSKBotObjectiveDebugInfo& Info = Controller->GetObjectiveDebugInfo();
		const APawn* Pawn = Controller->GetPawn();
		if (!Info.bValid || !Pawn)
		{
			continue;
		}

		const FVector WorldPos = Pawn->GetActorLocation() + FVector(0.0f, 0.0f, HeadOffsetZ);
		if (FVector::Dist(ViewLocation, WorldPos) > MaxDrawDistance)
		{
			continue;
		}

		const FVector Screen = Canvas->Project(WorldPos);
		if (Screen.Z <= 0.0f)
		{
			continue;
		}

		TArray<FString> Lines;
		Lines.Add(FString::Printf(TEXT("Rank  %.2f"), Info.RankPressure));
		for (int32 Index = 0; Index < CSKBotObjectiveCount; ++Index)
		{
			const TCHAR* Marker = ((int32)Info.Current == Index) ? TEXT("> ") : TEXT("   ");
			Lines.Add(FString::Printf(TEXT("%s%s  %.2f"), Marker, ObjectiveNames[Index], Info.Scores[Index]));
		}

		float MaxWidth = 0.0f;
		float LineHeight = 0.0f;
		for (const FString& Line : Lines)
		{
			float Width = 0.0f;
			float Height = 0.0f;
			Canvas->TextSize(Font, Line, Width, Height);
			MaxWidth = FMath::Max(MaxWidth, Width);
			LineHeight = FMath::Max(LineHeight, Height);
		}

		const float PanelWidth = MaxWidth + PanelPadding * 2.0f;
		const float PanelHeight = LineHeight * Lines.Num() + PanelPadding * 2.0f;
		const float PanelX = Screen.X - PanelWidth * 0.5f;
		const float PanelY = Screen.Y - PanelHeight;

		FCanvasTileItem Panel(
			FVector2D(PanelX, PanelY),
			FVector2D(PanelWidth, PanelHeight),
			FLinearColor(0.0f, 0.0f, 0.0f, 0.6f));
		Panel.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(Panel);

		float TextY = PanelY + PanelPadding;
		for (int32 LineIndex = 0; LineIndex < Lines.Num(); ++LineIndex)
		{
			const FLinearColor Color = (LineIndex == 0)
				? FLinearColor::White
				: ObjectiveColors[LineIndex - 1];

			FCanvasTextItem TextItem(FVector2D(PanelX + PanelPadding, TextY), FText::FromString(Lines[LineIndex]), Font, Color);
			Canvas->DrawItem(TextItem);
			TextY += LineHeight;
		}
	}
}
