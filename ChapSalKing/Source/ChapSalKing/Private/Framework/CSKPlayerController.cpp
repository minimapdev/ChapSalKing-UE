#include "Framework/CSKPlayerController.h"

#include "Framework/CSKGameState.h"
#include "Engine/World.h"

void ACSKPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 입력 모드는 UGameViewportClient에 걸리고 뷰포트는 레벨 전환을 넘어 살아남는다.
	// 프론트엔드가 켜둔 UI 전용 모드를 여기서 되돌리지 않으면 인게임 입력이 통째로 무시된다
	if (IsLocalController())
	{
		bShowMouseCursor = false;
		bEnableClickEvents = false;
		SetInputMode(FInputModeGameOnly());
	}

	if (ACSKGameState* CSKGameState = GetCSKGameState())
	{
		ChapSalKingChangedHandle = CSKGameState->OnChapSalKingChanged.AddUObject(
			this, &ACSKPlayerController::HandleChapSalKingChanged);

		if (ACSKPlayerCharacter* CurrentKing = CSKGameState->GetChapSalKing())
		{
			HandleChapSalKingChanged(CurrentKing);
		}

		CSKGameState->OnMatchPhaseChanged.AddDynamic(
			this, &ACSKPlayerController::HandleMatchPhaseChanged);

		if (CSKGameState->IsMatchFinished())
		{
			EnterMatchResultInputMode();
		}
	}
}

void ACSKPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ACSKGameState* CSKGameState = GetCSKGameState())
	{
		CSKGameState->OnChapSalKingChanged.Remove(ChapSalKingChangedHandle);
		CSKGameState->OnMatchPhaseChanged.RemoveDynamic(
			this, &ACSKPlayerController::HandleMatchPhaseChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void ACSKPlayerController::HandleChapSalKingChanged(ACSKPlayerCharacter* NewKing)
{
	if (!IsLocalController())
	{
		return;
	}

	if (NewKing)
	{
		BP_OnChapSalKingAppeared(NewKing);
	}
	else
	{
		BP_OnChapSalKingCleared();
	}
}

void ACSKPlayerController::HandleMatchPhaseChanged(ECSKMatchPhase NewPhase)
{
	if (!IsLocalController())
	{
		return;
	}

	if (NewPhase == ECSKMatchPhase::Finished)
	{
		EnterMatchResultInputMode();
	}
}

void ACSKPlayerController::EnterMatchResultInputMode()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	SetInputMode(FInputModeGameAndUI());
}

ACSKGameState* ACSKPlayerController::GetCSKGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<ACSKGameState>() : nullptr;
}
