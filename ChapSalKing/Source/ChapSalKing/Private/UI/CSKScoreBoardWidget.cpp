#include "UI/CSKScoreBoardWidget.h"

#include "Framework/CSKGameState.h"
#include "Player/CSKPlayerCharacter.h"
#include "UI/CSKScoreRowWidget.h"

#include "Components/PanelWidget.h"
#include "GameFramework/PlayerState.h"

void UCSKScoreBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ACSKGameState* CSKGameState = GetCSKGameState();
	if (!CSKGameState)
	{
		return;
	}

	CSKGameState->OnScoreboardChanged.AddDynamic(
		this, &UCSKScoreBoardWidget::HandleScoreboardChanged);
	CSKGameState->OnChapSalKingChangedBP.AddDynamic(
		this, &UCSKScoreBoardWidget::HandleChapSalKingChanged);

	Rebuild();
}

void UCSKScoreBoardWidget::NativeDestruct()
{
	if (ACSKGameState* CSKGameState = GetCSKGameState())
	{
		CSKGameState->OnScoreboardChanged.RemoveDynamic(
			this, &UCSKScoreBoardWidget::HandleScoreboardChanged);
		CSKGameState->OnChapSalKingChangedBP.RemoveDynamic(
			this, &UCSKScoreBoardWidget::HandleChapSalKingChanged);
	}

	Super::NativeDestruct();
}

void UCSKScoreBoardWidget::HandleScoreboardChanged()
{
	Rebuild();
}

void UCSKScoreBoardWidget::HandleChapSalKingChanged(ACSKPlayerCharacter* NewKing)
{
	Rebuild();
}

void UCSKScoreBoardWidget::Rebuild()
{
	if (!RowWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] ScoreBoard: RowWidgetClass 미설정"));
		return;
	}

	const ACSKGameState* CSKGameState = GetCSKGameState();
	if (!CSKGameState)
	{
		return;
	}

	const ACSKPlayerCharacter* King = CSKGameState->GetChapSalKing();

	UCSKScoreRowWidget::PopulateRows(
		RowContainer,
		RowWidgetClass,
		GetOwningPlayer(),
		CSKGameState->GetPlayerScores(),
		GetOwningPlayerState(),
		King ? King->GetPlayerState() : nullptr,
		MaxRows);
}
