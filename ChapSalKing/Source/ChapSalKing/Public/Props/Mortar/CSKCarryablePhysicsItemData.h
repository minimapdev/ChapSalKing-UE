#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "Player/Components/CSKCarryVisualFollowComponent.h"
#include "CSKCarryablePhysicsItemData.generated.h"

USTRUCT(BlueprintType)
struct FCSKPhysicsLaunchSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launch")
	float ImpulseStrength = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launch")
	float ImpulseUp = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launch")
	float SpinStrength = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launch")
	float PickupLockout = 0.45f;
};

UCLASS(BlueprintType)
class CHAPSALKING_API UCSKCarryablePhysicsItemData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Carry Visual")
	FCSKCarryVisualFollowSettings CarryVisualFollowSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Launch")
	FCSKPhysicsLaunchSettings LaunchSettings;
};
