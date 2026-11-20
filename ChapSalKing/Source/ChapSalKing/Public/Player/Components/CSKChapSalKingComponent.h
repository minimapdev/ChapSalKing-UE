#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "Player/CSKPlayerComponentBase.h"
#include "Player/CSKPlayerStatData.h"
#include "Player/Components/CSKEatableComponent.h"
#include "Tteok/TteokData.h"
#include "CSKChapSalKingComponent.generated.h"

class ACSKGameState;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKChapSalKingComponent : public UCSKPlayerComponentBase
{
	GENERATED_BODY()

public:
	UCSKChapSalKingComponent();

	void InitializeChapSalKing();

	// 절구 진입 시 보유 재료에 맞는 찹쌀킹으로 즉시 변신
	void TransformToChapSalKing();

	bool IsChapSalKing() const
	{
		return bIsChapSalKing;
	}

	UTteokData* GetCurrentTteokData() const
	{
		return CurrentTteokData;
	}

	void RevertFromChapSalKing();

	// 킹 착지 임팩트: 실제 바닥 트레이스 지점에 슬램·셰이크·VFX·떡 착지 효과 일괄 재생 (도약 착지·달 복귀 공용)
	void PlayLandingImpact();

	void EatAttack(const FVector& AttackDirection);
	void NotifyDoughStuck(UCSKEatableComponent* EatableComponent);

	// 먹힌 반죽이 붙을 빈 스틱 소켓 예약(없으면 NAME_None). 멀티 확장 seam: 서버 권위 이관 지점
	FName ReserveStickSocket();
	void ReleaseStickSocket(FName SocketName);

	UFUNCTION(BlueprintPure, Category = "CSK|ChapSalKing|Session")
	float GetKingSessionDuration() const;

	UFUNCTION(BlueprintPure, Category = "CSK|ChapSalKing|Session")
	float GetKingSessionTimeRemaining() const;

	UFUNCTION(BlueprintPure, Category = "CSK|ChapSalKing|Session")
	float GetKingSessionTimeRemainingRatio() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|ChapSalKing")
	void BP_OnEatAttackStarted(const FVector& AttackDirection);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|ChapSalKing")
	void BP_OnEatTargetsFound(const TArray<UCSKEatableComponent*>& EatTargets);

private:
	TOptional<ETteokType> ResolveTteokTypeFromIngredient() const;

	void StartKingSession();
	void EndKingSession();
	void StartKingDurationTimer();
	void ClearKingDurationTimer();
	void OnKingDurationExpired();

	TArray<UCSKEatableComponent*> FindEatTargets(
		const FVector& AttackDirection,
		const FCSKPlayerEatAttackSettings& Settings,
		const FVector& SweepStart,
		const FVector& SweepEnd,
		float SweepRadius) const;
	bool CanEatTarget(
		const UCSKEatableComponent* TargetEatable,
		const FVector& AttackDirection,
		const FCSKPlayerEatAttackSettings& Settings) const;
	float CalculateEatTargetScore(const UCSKEatableComponent* TargetEatable, const FVector& AttackDirection) const;
	void HandleEatTargetConsumed(UCSKEatableComponent* EatableComponent, const FCSKEatContext& EatContext);

	void CancelOngoingEatReactions();

	static FName MakeStickSocketName(FName BaseName, int32 Index);

	const FCSKPlayerEatAttackSettings& GetEatAttackSettings() const;
	FVector GetEatSweepStart(const FVector& AttackDirection, const FCSKPlayerEatAttackSettings& Settings) const;
	FVector GetEatSweepEnd(const FVector& AttackDirection, const FCSKPlayerEatAttackSettings& Settings) const;
	float GetOwnerAppliedScale() const;
	float GetEatScaleMultiplier(float ScalePower) const;
	float GetScaledEatSweepRadius(const FCSKPlayerEatAttackSettings& Settings) const;
	float GetScaledEatSweepLength(const FCSKPlayerEatAttackSettings& Settings) const;
	float GetScaledEatSweepStartOffset(const FCSKPlayerEatAttackSettings& Settings) const;
	float GetScaledEatHeightOffset(const FCSKPlayerEatAttackSettings& Settings) const;
	ACSKGameState* GetCSKGameState() const;

	bool bIsChapSalKing = false;

	UPROPERTY()
	TObjectPtr<UTteokData> CurrentTteokData;

	FTimerHandle KingDurationTimerHandle;

	// 현재 점유 중인 스틱 소켓 이름들
	TSet<FName> OccupiedStickSockets;
};
