#pragma once

#include "CoreMinimal.h"
#include "CSKCombatTypes.generated.h"

USTRUCT(BlueprintType)
struct FCSKCombatHitContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Combat")
	TObjectPtr<AActor> Attacker = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Combat")
	TObjectPtr<AActor> Victim = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Combat")
	FVector HitDirection = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Combat")
	float KnockbackStrength = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Combat")
	float DropFraction = 0.0f;
};
