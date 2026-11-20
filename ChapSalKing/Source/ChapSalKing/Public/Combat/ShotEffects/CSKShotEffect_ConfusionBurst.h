#pragma once

#include "CoreMinimal.h"
#include "Combat/ShotEffects/CSKShotEffect.h"
#include "CSKShotEffect_ConfusionBurst.generated.h"

UCLASS(meta = (DisplayName = "Confusion Burst"))
class CHAPSALKING_API UCSKShotEffect_ConfusionBurst : public UCSKShotEffect
{
	GENERATED_BODY()

public:
	virtual void OnCharacterHit(const FCSKShotEffectContext& Context) override;
	virtual void OnSurfaceImpact(const FCSKShotEffectContext& Context) override;

private:
	void Burst(const FCSKShotEffectContext& Context);

	UPROPERTY(EditAnywhere, Category = "Confuse", meta = (ClampMin = "0.0", Units = "s"))
	float Duration = 2.0f;
};
