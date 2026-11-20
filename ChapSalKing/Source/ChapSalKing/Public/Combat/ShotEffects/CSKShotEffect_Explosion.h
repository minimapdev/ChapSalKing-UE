#pragma once

#include "CoreMinimal.h"
#include "Combat/ShotEffects/CSKShotEffect.h"
#include "CSKShotEffect_Explosion.generated.h"

UCLASS(meta = (DisplayName = "Explosion"))
class CHAPSALKING_API UCSKShotEffect_Explosion : public UCSKShotEffect
{
	GENERATED_BODY()

public:
	virtual void OnCharacterHit(const FCSKShotEffectContext& Context) override;
	virtual void OnSurfaceImpact(const FCSKShotEffectContext& Context) override;

private:
	void Explode(const FCSKShotEffectContext& Context);

	UPROPERTY(EditAnywhere, Category = "Explosion", meta = (ClampMin = "0.0"))
	float Radius = 350.0f;

	UPROPERTY(EditAnywhere, Category = "Explosion", meta = (ClampMin = "0.0"))
	float KnockbackStrength = 1400.0f;

	UPROPERTY(EditAnywhere, Category = "Explosion", meta = (ClampMin = "0.0"))
	float KnockbackUpwardStrength = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Explosion", meta = (ClampMin = "0.0"))
	float TargetScaleResistancePower = 0.5f;
};
