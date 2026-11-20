#pragma once

#include "CoreMinimal.h"
#include "CSKAffinityTypes.generated.h"

// 계열 — 재료와 떡이 공유하는 정체성. 같은 계열끼리는 상태·시야류 효과가 안 통함 (꿀은 역이용 버프)
UENUM(BlueprintType)
enum class ECSKAffinity : uint8
{
	None,
	RedBean,     // 팥 / 찹쌀떡
	SoyPowder,   // 콩가루 / 인절미
	Honey,       // 꿀 / 꿀떡
	Rainbow      // 오색 / 무지개떡
};
