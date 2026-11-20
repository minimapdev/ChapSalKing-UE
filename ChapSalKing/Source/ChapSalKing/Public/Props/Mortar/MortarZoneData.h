#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "MortarZoneData.generated.h"

class UNiagaraSystem;

UCLASS(BlueprintType)
class CHAPSALKING_API UMortarZoneData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Rise", meta = (ClampMin = "0.01", Units = "s"))
	float RiseDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Rise", meta = (ClampMin = "1.0"))
	float RiseEaseExponent = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Spin", meta = (ClampMin = "0.01", Units = "s"))
	float SpinDuration = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Spin", meta = (ClampMin = "1"))
	int32 SpinTurns = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Spin", meta = (ClampMin = "1.0"))
	float SpinEaseExponent = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Strike", meta = (ClampMin = "0.01", Units = "s"))
	float StrikeDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Strike", meta = (ClampMin = "1.0"))
	float StrikeEaseExponent = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Effect")
	TObjectPtr<UNiagaraSystem> TransformEffect;

	// 변신 이펙트 시작 후 복귀 비행 발사까지 대기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Return", meta = (ClampMin = "0.0", Units = "s"))
	float ReturnDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Return", meta = (ClampMin = "0.05", Units = "s"))
	float ReturnFlightDuration = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Return", meta = (ClampMin = "0.0", Units = "cm"))
	float ReturnArcHeight = 1500.0f;
};
