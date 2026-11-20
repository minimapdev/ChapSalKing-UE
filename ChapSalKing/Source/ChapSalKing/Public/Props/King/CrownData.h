#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "Player/Components/CSKCarryVisualFollowComponent.h"
#include "CrownData.generated.h"

class UAnimSequence;
class USkeletalMesh;

UCLASS(BlueprintType)
class CHAPSALKING_API UCrownData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crown|Visual")
	TObjectPtr<USkeletalMesh> CrownMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crown|Animation")
	TObjectPtr<UAnimSequence> CrownTransitionAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crown|Carry Visual")
	FCSKCarryVisualFollowSettings CarryVisualFollowSettings;
};
