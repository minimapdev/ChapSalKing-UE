#include "Combat/CSKCombatStatics.h"

#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerStatusComponent.h"
#include "Combat/CSKCombatHitReceiverInterface.h"

#include "GameFramework/Character.h"

FCSKCombatHitContext FCSKCombatStatics::ApplyKnockbackHit(
	AActor* Attacker,
	AActor* Victim,
	const FVector& Direction,
	float KnockbackStrength,
	float KnockbackUpwardStrength,
	float TargetScaleResistancePower,
	float DropFraction
)
{
	float FinalKnockback = KnockbackStrength;

	if (ACharacter* HitCharacter = Cast<ACharacter>(Victim))
	{
		float Resistance = 1.0f;
		bool bKnockbackImmune = false;
		if (const ACSKPlayerCharacter* HitCSKCharacter = Cast<ACSKPlayerCharacter>(HitCharacter))
		{
			Resistance = HitCSKCharacter->GetKnockbackResistance(TargetScaleResistancePower);
			bKnockbackImmune = HitCSKCharacter->GetStatusComponent()->IsImmuneToKnockback();
		}

		FinalKnockback = KnockbackStrength / Resistance;

		if (!bKnockbackImmune && (KnockbackStrength > 0.0f || KnockbackUpwardStrength > 0.0f))
		{
			const FVector LaunchVelocity =
				Direction * FinalKnockback
				+ FVector(0.0f, 0.0f, KnockbackUpwardStrength / Resistance);

			HitCharacter->LaunchCharacter(LaunchVelocity, true, true);
		}
	}

	FCSKCombatHitContext HitContext;
	HitContext.Attacker = Attacker;
	HitContext.Victim = Victim;
	HitContext.HitDirection = Direction;
	HitContext.KnockbackStrength = FinalKnockback;
	HitContext.DropFraction = DropFraction;

	if (Victim && Victim->GetClass()->ImplementsInterface(UCSKCombatHitReceiverInterface::StaticClass()))
	{
		ICSKCombatHitReceiverInterface::Execute_ReceiveCombatHit(Victim, HitContext);
	}

	return HitContext;
}
