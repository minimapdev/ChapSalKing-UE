#include "Framework/CSKGameState.h"
#include "Framework/CSKPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Player/CSKPlayerCharacter.h"

TArray<FCSKPlayerScoreEntry> ACSKGameState::GetPlayerScores() const
{
	TArray<FCSKPlayerScoreEntry> Entries;

	for (APlayerState* PS : PlayerArray)
	{
		const ACSKPlayerState* CSKPS = Cast<ACSKPlayerState>(PS);
		if (!CSKPS)
		{
			continue;
		}

		FCSKPlayerScoreEntry Entry;
		Entry.PlayerName = CSKPS->GetPlayerName();
		Entry.Score = CSKPS->GetCSKScore();
		Entry.PlayerState = PS;
		Entries.Add(Entry);
	}

	Entries.Sort([](const FCSKPlayerScoreEntry& A, const FCSKPlayerScoreEntry& B)
		{
			return A.Score > B.Score;
		});

	return Entries;
}

void ACSKGameState::NotifyScoreChanged()
{
	OnScoreboardChanged.Broadcast();
}

void ACSKGameState::SetChapSalKing(ACSKPlayerCharacter* NewKing)
{
	CurrentChapSalKing = NewKing;
	OnChapSalKingChanged.Broadcast(NewKing);
	OnChapSalKingChangedBP.Broadcast(NewKing);
}

void ACSKGameState::ClearChapSalKing(ACSKPlayerCharacter* King)
{
	if (CurrentChapSalKing != King)
	{
		return;
	}

	CurrentChapSalKing = nullptr;
	OnChapSalKingChanged.Broadcast(nullptr);
	OnChapSalKingChangedBP.Broadcast(nullptr);
}

float ACSKGameState::GetMatchTimeRemaining() const
{
	if (MatchPhase != ECSKMatchPhase::InProgress)
	{
		return 0.0f;
	}
	return FMath::Max(0.0f, MatchEndServerTime - GetServerWorldTimeSeconds());
}

void ACSKGameState::BroadcastMatchEvent(const FCSKMatchEventInfo& EventInfo)
{
	OnMatchEvent.Broadcast(EventInfo);
}

void ACSKGameState::StartMatch(float MatchDuration)
{
	MatchEndServerTime = GetServerWorldTimeSeconds() + MatchDuration;
	MatchPhase = ECSKMatchPhase::InProgress;
	OnMatchPhaseChanged.Broadcast(MatchPhase);
}

void ACSKGameState::FinishMatch(const TArray<FCSKPlayerScoreEntry>& Results)
{
	if (MatchPhase == ECSKMatchPhase::Finished)
	{
		return;
	}
	FinalResults = Results;
	MatchPhase = ECSKMatchPhase::Finished;
	OnMatchPhaseChanged.Broadcast(MatchPhase);
}
