#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "Combat/CSKAffinityTypes.h"
#include "HoneyZoneData.generated.h"

UCLASS(BlueprintType)
class CHAPSALKING_API UHoneyZoneData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone", meta = (ClampMin = "0.0"))
	float ZoneRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SlowFactor = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone", meta = (ClampMin = "1.0"))
	float HoneyHolderSpeedFactor = 1.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone", meta = (ClampMin = "0.0", Units = "s"))
	float LifeSeconds = 5.0f;

	// 장판의 계열 — 같은 계열(꿀 재료 반죽, 꿀떡 킹)은 둔화 대신 가속
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone")
	ECSKAffinity ZoneAffinity = ECSKAffinity::Honey;
};
