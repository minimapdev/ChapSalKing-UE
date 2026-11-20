#include "Combat/ShotEffects/CSKShotEffect_ConfusionBurst.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerStatusComponent.h"

#include "Engine/World.h"
#include "Engine/OverlapResult.h"

void UCSKShotEffect_ConfusionBurst::OnCharacterHit(const FCSKShotEffectContext& Context)
{
	Burst(Context);
}

void UCSKShotEffect_ConfusionBurst::OnSurfaceImpact(const FCSKShotEffectContext& Context)
{
	Burst(Context);
}

void UCSKShotEffect_ConfusionBurst::Burst(const FCSKShotEffectContext& Context)
{
	if (!Context.World || Context.AreaRadius <= 0.0f)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	Context.World->OverlapMultiByObjectType(
		Overlaps,
		Context.HitLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Context.AreaRadius)
	);

	TSet<AActor*> Processed;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor || Actor == Context.Attacker || Processed.Contains(Actor))
		{
			continue;
		}
		Processed.Add(Actor);

		ACSKPlayerCharacter* Victim = Cast<ACSKPlayerCharacter>(Actor);
		if (!Victim)
		{
			continue;
		}

		Victim->GetStatusComponent()->TryApplyStatus(
			ECSKStatusType::Confusion, Duration, Context.SourceAffinity);
	}
}
