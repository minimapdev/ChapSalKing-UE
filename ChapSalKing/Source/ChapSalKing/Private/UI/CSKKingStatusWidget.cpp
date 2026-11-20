#include "UI/CSKKingStatusWidget.h"

#include "Framework/CSKGameState.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKChapSalKingComponent.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UCSKKingStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ACSKGameState* CSKGameState = GetCSKGameState();
	if (!CSKGameState)
	{
		return;
	}

	CSKGameState->OnChapSalKingChangedBP.AddDynamic(
		this, &UCSKKingStatusWidget::HandleChapSalKingChanged);

	ApplyKing(CSKGameState->GetChapSalKing());
}

void UCSKKingStatusWidget::NativeDestruct()
{
	if (ACSKGameState* CSKGameState = GetCSKGameState())
	{
		CSKGameState->OnChapSalKingChangedBP.RemoveDynamic(
			this, &UCSKKingStatusWidget::HandleChapSalKingChanged);
	}

	Super::NativeDestruct();
}

void UCSKKingStatusWidget::HandleChapSalKingChanged(ACSKPlayerCharacter* NewKing)
{
	ApplyKing(NewKing);
}

void UCSKKingStatusWidget::ApplyKing(ACSKPlayerCharacter* NewKing)
{
	if (!NewKing)
	{
		SetVisibility(ESlateVisibility::Collapsed);

		bUrgentDisplayed = false;
		BP_OnKingCleared();
		return;
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);

	const APlayerState* KingPlayerState = NewKing->GetPlayerState();
	KingNameText->SetText(KingPlayerState
		? FText::FromString(KingPlayerState->GetPlayerName())
		: FText::GetEmpty());

	bUrgentDisplayed = false;
	BP_OnKingAppeared(
		NewKing,
		KingPlayerState == GetOwningPlayerState(),
		NewKing->GetCurrentTteokData());
}

void UCSKKingStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const ACSKGameState* CSKGameState = GetCSKGameState();
	const ACSKPlayerCharacter* King = CSKGameState ? CSKGameState->GetChapSalKing() : nullptr;
	if (!King)
	{
		return;
	}

	const UCSKChapSalKingComponent* KingComponent = King->GetChapSalKingComponent();
	if (!KingComponent)
	{
		return;
	}

	const float Ratio = KingComponent->GetKingSessionTimeRemainingRatio();
	SessionBar->SetPercent(Ratio);

	const bool bNowUrgent = Ratio <= UrgentRatio;
	if (bNowUrgent != bUrgentDisplayed)
	{
		bUrgentDisplayed = bNowUrgent;
		BP_OnUrgentChanged(bUrgentDisplayed);
	}
}
