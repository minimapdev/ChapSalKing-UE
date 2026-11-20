#include "Combat/ShotEffects/CSKShotEffect_Explosion.h"
#include "Combat/CSKCombatStatics.h"
#include "Framework/CSKGameInstance.h"

#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"

void UCSKShotEffect_Explosion::OnCharacterHit(const FCSKShotEffectContext& Context)
{
	Explode(Context);
}

void UCSKShotEffect_Explosion::OnSurfaceImpact(const FCSKShotEffectContext& Context)
{
	Explode(Context);
}

void UCSKShotEffect_Explosion::Explode(const FCSKShotEffectContext& Context)
{
	if (!Context.World)
	{
		return;
	}

	if (UCSKGameInstance::IsDrawDebugEnabled(Context.World))
	{
		DrawDebugSphere(Context.World, Context.HitLocation, Radius, 24, FColor::Orange, false, 2.0f);
	}

	TArray<FOverlapResult> Overlaps;
	Context.World->OverlapMultiByObjectType(
		Overlaps,
		Context.HitLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Radius)
	);

	TSet<AActor*> Processed;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Victim = Overlap.GetActor();
		if (!Victim || Victim == Context.Attacker || Processed.Contains(Victim))
		{
			continue;
		}
		Processed.Add(Victim);

		FVector Direction = Victim->GetActorLocation() - Context.HitLocation;
		Direction.Z = 0.0f;
		if (!Direction.Normalize())
		{
			Direction = FVector(Context.HitDirection.X, Context.HitDirection.Y, 0.0f).GetSafeNormal();
		}

		FCSKCombatStatics::ApplyKnockbackHit(
			Context.Attacker,
			Victim,
			Direction,
			KnockbackStrength,
			KnockbackUpwardStrength,
			TargetScaleResistancePower,
			0.0f
		);
	}
}
