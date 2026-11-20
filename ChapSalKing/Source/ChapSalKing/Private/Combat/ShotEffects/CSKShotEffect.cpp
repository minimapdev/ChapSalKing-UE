#include "Combat/ShotEffects/CSKShotEffect.h"

void UCSKShotEffect::DispatchCharacterHit(const TArray<TObjectPtr<UCSKShotEffect>>& Effects, const FCSKShotEffectContext& Context)
{
	for (UCSKShotEffect* Effect : Effects)
	{
		if (Effect)
		{
			Effect->OnCharacterHit(Context);
		}
	}
}

void UCSKShotEffect::DispatchSurfaceImpact(const TArray<TObjectPtr<UCSKShotEffect>>& Effects, const FCSKShotEffectContext& Context)
{
	for (UCSKShotEffect* Effect : Effects)
	{
		if (Effect)
		{
			Effect->OnSurfaceImpact(Context);
		}
	}
}
