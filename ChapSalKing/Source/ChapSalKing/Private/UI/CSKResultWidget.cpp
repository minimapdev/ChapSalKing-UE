#include "UI/CSKResultWidget.h"

#include "UI/CSKScoreRowWidget.h"

#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UCSKResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RestartButton->OnClicked.AddDynamic(this, &UCSKResultWidget::HandleRestartClicked);
	QuitButton->OnClicked.AddDynamic(this, &UCSKResultWidget::HandleQuitClicked);

	ACSKGameState* CSKGameState = GetCSKGameState();
	if (!CSKGameState)
	{
		return;
	}

	CSKGameState->OnMatchPhaseChanged.AddDynamic(
		this, &UCSKResultWidget::HandleMatchPhaseChanged);

	ApplyPhase(CSKGameState->GetMatchPhase());
}

void UCSKResultWidget::NativeDestruct()
{
	RestartButton->OnClicked.RemoveDynamic(this, &UCSKResultWidget::HandleRestartClicked);
	QuitButton->OnClicked.RemoveDynamic(this, &UCSKResultWidget::HandleQuitClicked);

	if (ACSKGameState* CSKGameState = GetCSKGameState())
	{
		CSKGameState->OnMatchPhaseChanged.RemoveDynamic(
			this, &UCSKResultWidget::HandleMatchPhaseChanged);
	}

	Super::NativeDestruct();
}

void UCSKResultWidget::HandleMatchPhaseChanged(ECSKMatchPhase NewPhase)
{
	ApplyPhase(NewPhase);
}

void UCSKResultWidget::ApplyPhase(ECSKMatchPhase Phase)
{
	if (Phase == ECSKMatchPhase::Finished)
	{
		ShowResults();
		return;
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void UCSKResultWidget::ShowResults()
{
	// 버튼을 눌러야 하므로 클릭을 받는 가시성으로
	SetVisibility(ESlateVisibility::Visible);

	if (!RowWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] Result: RowWidgetClass 미설정"));
	}
	else if (const ACSKGameState* CSKGameState = GetCSKGameState())
	{
		// 결과는 확정된 순위이므로 킹 표시는 하지 않는다
		UCSKScoreRowWidget::PopulateRows(
			RowContainer,
			RowWidgetClass,
			GetOwningPlayer(),
			CSKGameState->GetFinalResults(),
			GetOwningPlayerState(),
			nullptr,
			0);
	}

	BP_OnResultShown();
}

void UCSKResultWidget::HandleRestartClicked()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::OpenLevel(this, FName(*World->GetName()));
}

void UCSKResultWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
