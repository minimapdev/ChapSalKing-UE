#pragma once

#include "CoreMinimal.h"
#include "Combat/CSKAffinityTypes.h"
#include "CSKMatchEventTypes.generated.h"

class APlayerState;

// 전황 알림의 종류. 문구 조립은 UI가 하고, 여기서는 무슨 일이 누구에게 일어났는지만 넘긴다
UENUM(BlueprintType)
enum class ECSKMatchEventType : uint8
{
	KingAppeared,
	KingEnded,
	DoughEaten,
	DoughEscaped,
	SwallowedBySnake,
	ItemAcquired,
	PestleRespawned,
	PouchDropped
};

/**
 * 전황 한 건. 피드 위젯이 이걸 받아 한 줄을 만든다.
 *
 * EventInstigator는 "사건의 주인공"이다. 가해자가 있는 사건(먹힘)은 가해자가, 가해자가 없는
 * 사건(뱀에 삼켜짐·재료 획득)은 당사자가 온다. 어느 쪽인지는 EventType이 정하므로
 * 두 칸에 역할을 더 만들지 않는다.
 */
USTRUCT(BlueprintType)
struct FCSKMatchEventInfo
{
	GENERATED_BODY()

	FCSKMatchEventInfo() = default;

	explicit FCSKMatchEventInfo(
		ECSKMatchEventType InEventType,
		APlayerState* InInstigator = nullptr,
		APlayerState* InTarget = nullptr)
		: EventType(InEventType)
		, EventInstigator(InInstigator)
		, EventTarget(InTarget)
	{
	}

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Match")
	ECSKMatchEventType EventType = ECSKMatchEventType::KingAppeared;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Match")
	TObjectPtr<APlayerState> EventInstigator = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Match")
	TObjectPtr<APlayerState> EventTarget = nullptr;

	// 플레이어가 아닌 대상의 이름(재료명 등). 문구의 {2}로 들어간다
	UPROPERTY(BlueprintReadOnly, Category = "CSK|Match")
	FText SubjectName;

	// 계열이 있는 사건만 채운다. 있으면 행 색이 계열 색으로 바뀐다
	UPROPERTY(BlueprintReadOnly, Category = "CSK|Match")
	ECSKAffinity Affinity = ECSKAffinity::None;
};
