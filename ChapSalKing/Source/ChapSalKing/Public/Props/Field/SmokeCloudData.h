#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "Combat/CSKAffinityTypes.h"
#include "SmokeCloudData.generated.h"

UCLASS(BlueprintType)
class CHAPSALKING_API USmokeCloudData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	// 연막 구 반경 (시야 차단 + 실명 펄스 공용. 떡 착지 스폰이면 착지 반경으로 덮어써짐)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Smoke", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1000.0"))
	float BlockRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Smoke", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "10.0", Units = "s"))
	float LifeSeconds = 4.0f;

	// 안의 캐릭터에게 실명 펄스를 주는 주기 (스폰 즉시 1회 + 반복)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Smoke", meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "2.0", Units = "s"))
	float PulseInterval = 0.5f;

	// 펄스당 실명 지속 (주기보다 길게 — 안에 있는 동안 끊김 없이, 나가면 이 시간 뒤 자동 해제)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Smoke", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3.0", Units = "s"))
	float ObstructDuration = 0.7f;

	// 연막의 계열 — 같은 계열(콩가루 반죽, 인절미 킹)은 실명 면역
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Smoke")
	ECSKAffinity CloudAffinity = ECSKAffinity::SoyPowder;
};
