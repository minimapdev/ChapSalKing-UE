#pragma once

#include "CoreMinimal.h"
#include "Combat/CSKCombatTypes.h"

class AActor;

class CHAPSALKING_API FCSKCombatStatics
{
public:
	static FCSKCombatHitContext ApplyKnockbackHit(
		AActor* Attacker,
		AActor* Victim,
		const FVector& Direction,
		float KnockbackStrength,
		float KnockbackUpwardStrength,
		float TargetScaleResistancePower,
		float DropFraction
	);
};
