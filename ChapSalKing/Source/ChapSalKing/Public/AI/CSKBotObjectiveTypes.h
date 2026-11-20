#pragma once

#include "CoreMinimal.h"
#include "CSKBotObjectiveTypes.generated.h"

// 반죽 봇의 이동 목표. SelectBotObjective가 Utility 점수로 선택해 블랙보드에 기록
UENUM(BlueprintType)
enum class ECSKBotObjective : uint8
{
	Wander,
	Flee,
	PickJelly,
	Fight,
	Farm,
	Pestle
};

constexpr int32 CSKBotObjectiveCount = 6;

// 블랙보드 키 이름의 단일 출처 — BT 노드 밖(컨트롤러 등)에서 키를 쓸 때 이 상수를 사용
namespace CSKBlackboardKeys
{
	inline const FName KingActor(TEXT("KingActor"));
	inline const FName SelfIsKing(TEXT("bSelfIsKing"));
}

// 봇 로직 정지 사유 — ACSKBotAIController::PushLogicPause/PopLogicPause에 넘긴다
namespace CSKLogicPauseReasons
{
	inline const FName Staggered(TEXT("Staggered"));
	inline const FName BeingEaten(TEXT("BeingEaten"));
	inline const FName SequenceLock(TEXT("SequenceLock"));
}

// 디버그 오버레이용 스냅샷
struct FCSKBotObjectiveDebugInfo
{
	bool bValid = false;
	ECSKBotObjective Current = ECSKBotObjective::Wander;
	float Scores[CSKBotObjectiveCount] = {};
	float RankPressure = 0.0f;
};
