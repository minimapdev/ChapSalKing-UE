#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "CSKBotAIConfigData.generated.h"

// 롤/보간 겸용 범위 값.
USTRUCT(BlueprintType)
struct FCSKFloatRange
{
	GENERATED_BODY()

	FCSKFloatRange() = default;

	FCSKFloatRange(float InMin, float InMax)
		: Min(InMin), Max(InMax)
	{
	}

	UPROPERTY(EditAnywhere)
	float Min = 0.0f;

	UPROPERTY(EditAnywhere)
	float Max = 1.0f;

	float Roll() const
	{
		return FMath::FRandRange(Min, Max);
	}

	float LerpBy(float Alpha) const
	{
		return FMath::Lerp(Min, Max, Alpha);
	}
};

// 봇 1명의 성격 스폰 시 1회 롤.
// TODO:멀티에서 클라가 성격을 알아야 하면 그때 UPROPERTY+복제로 승격
USTRUCT()
struct FCSKBotPersonality
{
	GENERATED_BODY()

	// 판단 성향
	float Aggression = 0.5f;   // 전투 목표 가중치
	float Greed = 0.5f;        // 젤리/파밍 가중치
	float Ambition = 0.5f;     // 절구공이 가중치

	// 전투 실력
	float Accuracy = 0.5f;
	float Reflex = 0.5f;

	// 행동 개인차
	float TimingScale = 1.0f;
	float DashScale = 1.0f;
	float JumpScale = 1.0f;

	// 킹 몸에 붙었을 때 자동 몸부림 속도(좌우 전환 횟수/초). 사람 연타와 겹치는 범위로 둔다
	float StruggleFlipsPerSecond = 4.5f;

	// 포위 위치 개인차
	FVector SurroundOffset = FVector::ZeroVector;
};

UCLASS()
class CHAPSALKING_API UCSKBotAIConfigData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	FCSKBotPersonality RollPersonality() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2.0"))
	float StaggerDuration = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Attack", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1000.0"))
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Attack", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3.0"))
	float AttackInterval = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Attack", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2.0"))
	float AttackIntervalRandom = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Dash", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float DashChanceNear = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Dash", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float DashChanceFar = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Dash", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3000.0"))
	float DashFarDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Dash", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3000.0"))
	float ThreatRange = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Dash", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float EscapeDashChance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Jump", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float JumpChance = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Locomotion", meta = (ClampMin = "1.0", UIMin = "1.0", UIMax = "3000.0"))
	float LocomotionUrgencyDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|King", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1000.0"))
	float KingEatRange = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|King", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "5000.0"))
	float KingGiveUpRangeMin = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|King", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "5000.0"))
	float KingGiveUpRangeMax = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|King", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2000.0"))
	float KingTargetSwitchMarginMin = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|King", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2000.0"))
	float KingTargetSwitchMarginMax = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|King", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1000.0"))
	float KingFacingScoreWeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|King", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3000.0"))
	float KingDangerRangeMin = 700.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|King", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3000.0"))
	float KingDangerRangeMax = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|King", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3000.0"))
	float DoughFleeDistance = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|King", meta = (ClampMin = "1.0", UIMin = "1.0", UIMax = "3.0"))
	float DoughFleeExitMultiplier = 1.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|King", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "5000.0"))
	float DoughWanderRadius = 1500.0f;

	// --- 성격 롤 범위 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Personality")
	FCSKFloatRange AggressionRange = FCSKFloatRange(0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Personality")
	FCSKFloatRange GreedRange = FCSKFloatRange(0.2f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Personality")
	FCSKFloatRange AmbitionRange = FCSKFloatRange(0.2f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Personality")
	FCSKFloatRange AccuracyRange = FCSKFloatRange(0.3f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Personality")
	FCSKFloatRange ReflexRange = FCSKFloatRange(0.3f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Personality")
	FCSKFloatRange TimingScaleRange = FCSKFloatRange(0.75f, 1.25f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Personality")
	FCSKFloatRange DashScaleRange = FCSKFloatRange(0.5f, 1.5f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Personality")
	FCSKFloatRange JumpScaleRange = FCSKFloatRange(0.4f, 1.6f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Personality")
	FCSKFloatRange SurroundRadiusRange = FCSKFloatRange(150.0f, 300.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Personality")
	FCSKFloatRange StruggleFlipsPerSecondRange = FCSKFloatRange(3.5f, 5.5f);

	// --- Utility 목표 선택 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3.0"))
	float FleeWeight = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3.0"))
	float JellyWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3.0"))
	float FightWeight = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3.0"))
	float FarmWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "0.5"))
	float WanderBaseScore = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float JellyRiskAversion = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "4.0"))
	float SizeAdvantagePower = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "5000.0"))
	float JellySearchRadius = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "5000.0"))
	float IngredientSearchRadius = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "5000.0"))
	float FightSearchRadius = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3.0"))
	float PestleWeight = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "10000.0"))
	float PestleSearchRadius = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective")
	FCSKFloatRange PestleNoticeDelayRange = FCSKFloatRange(0.3f, 2.5f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "1.0", UIMin = "1.0", UIMax = "2.0"))
	float CommitBonus = 1.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Objective", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3.0", Units = "s"))
	float MinCommitSeconds = 0.5f;

	// --- 사격 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Shoot", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "5000.0"))
	float ShootRange = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Shoot", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3.0"))
	float ShootInterval = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Shoot", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2.0"))
	float ShootIntervalRandom = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Shoot", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float LeadFactor = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Shoot", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "15.0"))
	float AimErrorHalfAngleDeg = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Shoot", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float ShootStandoffRatio = 0.8f;

	// --- 투사체 회피 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Dodge", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3000.0"))
	float DodgeDetectRange = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Dodge", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0", Units = "s"))
	float DodgeReactionDelay = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Dodge", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float DodgeChance = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Dodge", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float DodgeConeDotThreshold = 0.5f;

	// --- 킹 봇 슈퍼점프 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|KingLeap", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3000.0"))
	float KingLeapMinDistance = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|KingLeap", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "5.0", Units = "s"))
	float KingLeapDecisionInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|KingLeap", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float KingLeapChance = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|KingLeap", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3.0", Units = "s"))
	float KingLeapPredictSeconds = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|KingLeap", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2000.0"))
	float KingLeapClusterRadius = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|KingLeap", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3.0"))
	float KingLeapChaseWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|KingLeap", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
	float KingLeapClusterScorePerDough = 0.45f;

	// --- 고착 탈출 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Recovery", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "5.0", Units = "s"))
	float StuckSecondsThreshold = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Recovery", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "200.0"))
	float StuckMoveTolerance = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Recovery", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1000.0"))
	float StuckGoalIgnoreRadius = 200.0f;

	// --- 순위 압박 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Rank")
	FCSKFloatRange RankFleeScale = FCSKFloatRange(1.3f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Rank")
	FCSKFloatRange RankJellyScale = FCSKFloatRange(1.0f, 1.6f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Rank")
	FCSKFloatRange RankFightScale = FCSKFloatRange(0.8f, 1.4f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Rank")
	FCSKFloatRange RankPestleScale = FCSKFloatRange(0.9f, 1.5f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot|Rank", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float RankRiskTolerance = 0.5f;
};
