#include "Framework/CSKPlayerState.h"

#include "Framework/CSKGameState.h"

#include "Engine/World.h"

void ACSKPlayerState::SetCSKScore(int32 NewScore)
{
	if (CSKScore == NewScore)
	{
		return;
	}
	CSKScore = NewScore;

	if (const UWorld* World = GetWorld())
	{
		if (ACSKGameState* CSKGameState = World->GetGameState<ACSKGameState>())
		{
			CSKGameState->NotifyScoreChanged();
		}
	}
}
