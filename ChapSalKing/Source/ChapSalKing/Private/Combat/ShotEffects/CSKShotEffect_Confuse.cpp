#include "Combat/ShotEffects/CSKShotEffect_Confuse.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerStatusComponent.h"

void UCSKShotEffect_Confuse::OnCharacterHit(const FCSKShotEffectContext& Context)
{
	ACSKPlayerCharacter* Victim = Cast<ACSKPlayerCharacter>(Context.Victim);
	if (!Victim)
	{
		return;
	}

	Victim->GetStatusComponent()->TryApplyStatus(
		ECSKStatusType::Confusion, Duration, Context.SourceAffinity);
}
