#pragma once

#include "CoreMinimal.h"
#include "Combat/ShotEffects/CSKShotEffect.h"
#include "CSKShotEffect_SpawnActor.generated.h"

UCLASS(meta = (DisplayName = "Spawn Actor"))
class CHAPSALKING_API UCSKShotEffect_SpawnActor : public UCSKShotEffect
{
	GENERATED_BODY()

public:
	virtual void OnCharacterHit(const FCSKShotEffectContext& Context) override;
	virtual void OnSurfaceImpact(const FCSKShotEffectContext& Context) override;

private:
	void Spawn(const FCSKShotEffectContext& Context);

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AActor> ActorClass;
};
