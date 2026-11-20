#pragma once

#include "CoreMinimal.h"
#include "CSKShotTypes.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FCSKShotVFX
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot|VFX")
	TObjectPtr<UNiagaraSystem> MuzzleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot|VFX")
	TObjectPtr<UNiagaraSystem> ProjectileSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot|VFX")
	TObjectPtr<UNiagaraSystem> HitSystem;
};

USTRUCT(BlueprintType)
struct FCSKShotParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot")
	float Speed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot")
	float LifeSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot")
	float KnockbackStrength = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot")
	float KnockbackUpwardStrength = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropFraction = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot", meta = (ClampMin = "0.0", Units = "s"))
	float FireCooldown = 0.45f;

	// 명중 대상 몸집에 따른 넉백 저항 지수 (근접의 BodySlamTargetScaleResistancePower와 동일 역할)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot", meta = (ClampMin = "0.0"))
	float TargetScaleResistancePower = 0.5f;
};

USTRUCT(BlueprintType)
struct FCSKShotPattern
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot", meta = (ClampMin = "1"))
	int32 PelletCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot", meta = (ClampMin = "0.0"))
	float SpreadHalfAngleDeg = 0.0f;

};
