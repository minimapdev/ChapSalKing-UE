#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "BeanWallData.generated.h"

UCLASS(BlueprintType)
class CHAPSALKING_API UBeanWallData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall", meta = (ClampMin = "4", UIMin = "4", UIMax = "24"))
	int32 SegmentCount = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "500.0"))
	float WallHeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "10.0", Units = "s"))
	float LifeSeconds = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "0.5", Units = "s"))
	float CollisionEnableDelay = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "200.0", Units = "cm"))
	float BounceTriggerPadding = 30.0f;

	// 벽에 닿은 캐릭터를 중심(킹) 방향으로 밀어내는 세기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "3000.0"))
	float BounceStrength = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1000.0"))
	float BounceUpwardStrength = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2.0"))
	float TargetScaleResistancePower = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2.0", Units = "s"))
	float BounceCooldownSeconds = 0.4f;
};
