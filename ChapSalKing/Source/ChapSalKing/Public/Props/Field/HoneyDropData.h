#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "HoneyDropData.generated.h"

class AHoneyZone;

UCLASS(BlueprintType)
class CHAPSALKING_API UHoneyDropData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
	TSubclassOf<AHoneyZone> ZoneClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GroundNormalZThreshold = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "0.0", Units = "s"))
	float MaxLifeSeconds = 5.0f;
};
