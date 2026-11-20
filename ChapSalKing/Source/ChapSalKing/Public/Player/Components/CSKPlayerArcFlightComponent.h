#pragma once

#include "CoreMinimal.h"
#include "Player/CSKPlayerComponentBase.h"
#include "CSKPlayerArcFlightComponent.generated.h"

DECLARE_DELEGATE_OneParam(FCSKArcFlightFinished, const FVector& /*ArrivalLocation*/);

// 키네마틱 포물선 비행: 이동 물리 정지(Flying) 후 SetActorLocation 보간으로 아치 구동 — 목표에 무조건 도착.
// 킹 도약·널뛰기 발사·달 복귀가 공유한다. 동시에 한 비행만 유효.
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKPlayerArcFlightComponent : public UCSKPlayerComponentBase
{
	GENERATED_BODY()

public:
	UCSKPlayerArcFlightComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// bEndInFalling=false면 도착 후에도 Flying 유지 — 공중 연출(절구 안착 등)에서 낙하 방지
	void BeginArc(
		const FVector& Target,
		float Duration,
		float ArcHeight,
		bool bEndInFalling,
		FCSKArcFlightFinished OnFinished);

	void Cancel();

	bool IsFlying() const
	{
		return bFlying;
	}

private:
	void FinishArc();

	FVector ArcStart = FVector::ZeroVector;
	FVector ArcTarget = FVector::ZeroVector;
	float Elapsed = 0.0f;
	float TotalTime = 0.0f;
	float ArcPeak = 0.0f;
	FVector LastLocation = FVector::ZeroVector;
	bool bEndInFalling = true;
	bool bFlying = false;
	FCSKArcFlightFinished FinishedDelegate;
};
