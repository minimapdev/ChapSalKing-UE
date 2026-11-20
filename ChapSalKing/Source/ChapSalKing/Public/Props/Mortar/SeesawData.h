#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "SeesawData.generated.h"

UCLASS(BlueprintType)
class CHAPSALKING_API USeesawData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	// 탑승자가 아래로 내려와 있는 발판(Pivot1)으로 미끄러져 이동하는 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Board", meta = (ClampMin = "0.01", Units = "s"))
	float BoardDuration = 0.25f;

	// 널판의 기울기. Pivot1이 이 각도만큼 내려가 있고 Pivot2가 올라가 있는 것이 대기 상태
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Spring", meta = (ClampMin = "0.0", ClampMax = "80.0", Units = "deg"))
	float TiltAngle = 20.0f;

	// Pivot2가 슉 내려찍히며 Pivot1이 튀어오르는 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Spring", meta = (ClampMin = "0.01", Units = "s"))
	float SpringDuration = 0.35f;

	// 클수록 초반이 느리고 끝에서 급격히 튕긴다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Spring", meta = (ClampMin = "1.0"))
	float SpringEaseExponent = 3.0f;

	// 발사 후 널판이 대기 상태(Pivot1 내려감)로 되돌아오는 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Spring", meta = (ClampMin = "0.01", Units = "s"))
	float ResetDuration = 0.8f;

	// 달 절구까지의 포물선 비행 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Flight", meta = (ClampMin = "0.05", Units = "s"))
	float FlightDuration = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence|Flight", meta = (ClampMin = "0.0", Units = "cm"))
	float FlightArcHeight = 2500.0f;
};
