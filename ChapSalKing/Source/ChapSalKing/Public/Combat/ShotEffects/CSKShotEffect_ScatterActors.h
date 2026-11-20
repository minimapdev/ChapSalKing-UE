#pragma once

#include "CoreMinimal.h"
#include "Combat/ShotEffects/CSKShotEffect.h"
#include "CSKShotEffect_ScatterActors.generated.h"

UCLASS(meta = (DisplayName = "Scatter Actors"))
class CHAPSALKING_API UCSKShotEffect_ScatterActors : public UCSKShotEffect
{
	GENERATED_BODY()

public:
	virtual void OnCharacterHit(const FCSKShotEffectContext& Context) override;
	virtual void OnSurfaceImpact(const FCSKShotEffectContext& Context) override;

private:
	void Scatter(const FCSKShotEffectContext& Context);

	UPROPERTY(EditAnywhere, Category = "Scatter")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, Category = "Scatter", meta = (ClampMin = "1"))
	int32 Count = 5;

	UPROPERTY(EditAnywhere, Category = "Scatter", meta = (ClampMin = "0.0"))
	float HorizontalSpeedMin = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Scatter", meta = (ClampMin = "0.0"))
	float HorizontalSpeedMax = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Scatter", meta = (ClampMin = "0.0"))
	float UpwardSpeed = 450.0f;

	UPROPERTY(EditAnywhere, Category = "Scatter", meta = (ClampMin = "0.0"))
	float SpawnHeightOffset = 50.0f;
};
