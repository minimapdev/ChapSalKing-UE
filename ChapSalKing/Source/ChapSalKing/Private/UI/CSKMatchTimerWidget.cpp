#include "UI/CSKMatchTimerWidget.h"

#include "Components/TextBlock.h"

void UCSKMatchTimerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ACSKGameState* CSKGameState = GetCSKGameState();
	if (!CSKGameState)
	{
		return;
	}

	CSKGameState->OnMatchPhaseChanged.AddDynamic(
		this, &UCSKMatchTimerWidget::HandleMatchPhaseChanged);

	ApplyPhase(CSKGameState->GetMatchPhase());
}

void UCSKMatchTimerWidget::NativeDestruct()
{
	if (ACSKGameState* CSKGameState = GetCSKGameState())
	{
		CSKGameState->OnMatchPhaseChanged.RemoveDynamic(
			this, &UCSKMatchTimerWidget::HandleMatchPhaseChanged);
	}

	Super::NativeDestruct();
}

void UCSKMatchTimerWidget::HandleMatchPhaseChanged(ECSKMatchPhase NewPhase)
{
	ApplyPhase(NewPhase);
}

void UCSKMatchTimerWidget::ApplyPhase(ECSKMatchPhase Phase)
{
	switch (Phase)
	{
	case ECSKMatchPhase::WaitingToStart:
		SetVisibility(ESlateVisibility::Collapsed);
		break;

	case ECSKMatchPhase::InProgress:
		SetVisibility(ESlateVisibility::HitTestInvisible);
		break;

	case ECSKMatchPhase::Finished:
		SetVisibility(ESlateVisibility::HitTestInvisible);
		UpdateDisplay(0.0f);
		break;
	}
}

void UCSKMatchTimerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const ACSKGameState* CSKGameState = GetCSKGameState();
	if (!CSKGameState || !CSKGameState->IsMatchInProgress())
	{
		return;
	}

	UpdateDisplay(CSKGameState->GetMatchTimeRemaining());
}

void UCSKMatchTimerWidget::UpdateDisplay(float TimeRemaining)
{
	const int32 TotalSeconds = FMath::CeilToInt(TimeRemaining);
	if (TotalSeconds == DisplayedSeconds)
	{
		return;
	}
	DisplayedSeconds = TotalSeconds;

	TimeText->SetText(FText::FromString(
		FString::Printf(TEXT("%d:%02d"), TotalSeconds / 60, TotalSeconds % 60)));

	const bool bNowUrgent = TimeRemaining <= UrgentThreshold;
	if (bNowUrgent != bUrgentDisplayed)
	{
		bUrgentDisplayed = bNowUrgent;
		BP_OnUrgentChanged(bUrgentDisplayed);
	}
}
