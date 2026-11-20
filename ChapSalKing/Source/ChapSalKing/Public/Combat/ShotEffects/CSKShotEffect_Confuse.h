#pragma once

#include "CoreMinimal.h"
#include "Combat/ShotEffects/CSKShotEffect.h"
#include "CSKShotEffect_Confuse.generated.h"

UCLASS(meta = (DisplayName = "Confuse"))
class CHAPSALKING_API UCSKShotEffect_Confuse : public UCSKShotEffect
{
	GENERATED_BODY()

public:
	virtual void OnCharacterHit(const FCSKShotEffectContext& Context) override;

private:
	UPROPERTY(EditAnywhere, Category = "Confuse", meta = (ClampMin = "0.0", Units = "s"))
	float Duration = 2.0f;
};
