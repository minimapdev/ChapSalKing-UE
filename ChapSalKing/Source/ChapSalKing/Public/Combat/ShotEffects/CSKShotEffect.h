#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Combat/CSKAffinityTypes.h"
#include "CSKShotEffect.generated.h"

struct FCSKShotEffectContext
{
	AActor* Attacker = nullptr;
	AActor* Victim = nullptr;
	FVector HitLocation = FVector::ZeroVector;
	FVector HitDirection = FVector::ZeroVector;
	UWorld* World = nullptr;
	ECSKAffinity SourceAffinity = ECSKAffinity::None;
	float AreaRadius = 0.0f;
};

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, CollapseCategories)
class CHAPSALKING_API UCSKShotEffect : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnCharacterHit(const FCSKShotEffectContext& Context) {}
	virtual void OnSurfaceImpact(const FCSKShotEffectContext& Context) {}

	static void DispatchCharacterHit(const TArray<TObjectPtr<UCSKShotEffect>>& Effects, const FCSKShotEffectContext& Context);
	static void DispatchSurfaceImpact(const TArray<TObjectPtr<UCSKShotEffect>>& Effects, const FCSKShotEffectContext& Context);
};
