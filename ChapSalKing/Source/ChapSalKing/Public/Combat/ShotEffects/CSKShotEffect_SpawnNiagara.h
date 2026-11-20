#pragma once

#include "CoreMinimal.h"
#include "Combat/ShotEffects/CSKShotEffect.h"
#include "CSKShotEffect_SpawnNiagara.generated.h"

class UNiagaraSystem;

UCLASS(meta = (DisplayName = "Spawn Niagara"))
class CHAPSALKING_API UCSKShotEffect_SpawnNiagara : public UCSKShotEffect
{
	GENERATED_BODY()

public:
	virtual void OnCharacterHit(const FCSKShotEffectContext& Context) override;
	virtual void OnSurfaceImpact(const FCSKShotEffectContext& Context) override;

private:
	void Spawn(const FCSKShotEffectContext& Context);

	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<UNiagaraSystem> System;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	FVector Scale = FVector::OneVector;

	UPROPERTY(EditAnywhere, Category = "Niagara", meta = (Units = "cm"))
	float ZOffset = 15.0f;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	bool bUseAreaRadius = true;

	UPROPERTY(EditAnywhere, Category = "Niagara", meta = (EditCondition = "bUseAreaRadius"))
	FName RadiusParameterName = TEXT("User.Scale");

	UPROPERTY(
		EditAnywhere,
		Category = "Niagara",
		meta = (EditCondition = "bUseAreaRadius && bConvertRadiusToScale", ClampMin = "1.0", Units = "cm")
	)
	float BaseRadius = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Niagara", meta = (EditCondition = "bUseAreaRadius"))
	bool bConvertRadiusToScale = true;
};
