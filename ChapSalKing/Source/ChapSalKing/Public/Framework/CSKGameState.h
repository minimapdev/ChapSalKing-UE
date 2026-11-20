#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKMatchEventTypes.h"
#include "GameFramework/GameStateBase.h"
#include "CSKGameState.generated.h"

class ACSKPlayerCharacter;
class APlayerState;

USTRUCT(BlueprintType)
struct FCSKPlayerScoreEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Score")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Score")
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Score")
	TObjectPtr<APlayerState> PlayerState = nullptr;
};

UENUM(BlueprintType)
enum class ECSKMatchPhase : uint8
{
	WaitingToStart,
	InProgress,
	Finished
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnScoreboardChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchPhaseChanged, ECSKMatchPhase, NewPhase);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnChapSalKingChanged, ACSKPlayerCharacter* /*NewKing*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChapSalKingChangedDynamic, ACSKPlayerCharacter*, NewKing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCSKMatchEvent, const FCSKMatchEventInfo&, EventInfo);

UCLASS()
class CHAPSALKING_API ACSKGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "CSK|King")
	ACSKPlayerCharacter* GetChapSalKing() const
	{
		return CurrentChapSalKing;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|King")
	bool HasChapSalKing() const
	{
		return CurrentChapSalKing != nullptr;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Score")
	TArray<FCSKPlayerScoreEntry> GetPlayerScores() const;

	UPROPERTY(BlueprintAssignable, Category = "CSK|Score")
	FOnScoreboardChanged OnScoreboardChanged;

	UFUNCTION(BlueprintPure, Category = "CSK|Match")
	ECSKMatchPhase GetMatchPhase() const { return MatchPhase; }

	UFUNCTION(BlueprintPure, Category = "CSK|Match")
	bool IsMatchInProgress() const { return MatchPhase == ECSKMatchPhase::InProgress; }

	UFUNCTION(BlueprintPure, Category = "CSK|Match")
	bool IsMatchFinished() const { return MatchPhase == ECSKMatchPhase::Finished; }

	UFUNCTION(BlueprintPure, Category = "CSK|Match")
	float GetMatchTimeRemaining() const;

	UFUNCTION(BlueprintPure, Category = "CSK|Match")
	const TArray<FCSKPlayerScoreEntry>& GetFinalResults() const { return FinalResults; }

	UPROPERTY(BlueprintAssignable, Category = "CSK|Match")
	FOnMatchPhaseChanged OnMatchPhaseChanged;

	void StartMatch(float MatchDuration);
	void FinishMatch(const TArray<FCSKPlayerScoreEntry>& Results);

	void NotifyScoreChanged();
	void SetChapSalKing(ACSKPlayerCharacter* NewKing);
	void ClearChapSalKing(ACSKPlayerCharacter* King);

	void BroadcastMatchEvent(const FCSKMatchEventInfo& EventInfo);

	UPROPERTY(BlueprintAssignable, Category = "CSK|Match")
	FOnCSKMatchEvent OnMatchEvent;

	// C++ 내부(GameMode/PlayerController)용. BP·위젯은 아래 Dynamic 버전을 쓴다
	FOnChapSalKingChanged OnChapSalKingChanged;

	UPROPERTY(BlueprintAssignable, Category = "CSK|King")
	FOnChapSalKingChangedDynamic OnChapSalKingChangedBP;

private:
	UPROPERTY(Transient)
	TObjectPtr<ACSKPlayerCharacter> CurrentChapSalKing = nullptr;

	UPROPERTY(Transient)
	ECSKMatchPhase MatchPhase = ECSKMatchPhase::WaitingToStart;

	UPROPERTY(Transient)
	float MatchEndServerTime = 0.0f;

	UPROPERTY(Transient)
	TArray<FCSKPlayerScoreEntry> FinalResults;
};
