#pragma once

#include "CoreMinimal.h"
#include "Player/CSKPlayerComponentBase.h"
#include "Player/CSKPlayerStatData.h"
#include "CSKPlayerLeapComponent.generated.h"

class APlayerController;

UENUM()
enum class ELeapPhase : uint8
{
	None,
	Aiming,
	Leaping
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKPlayerLeapComponent : public UCSKPlayerComponentBase
{
	GENERATED_BODY()

public:
	UCSKPlayerLeapComponent();

	void InitializeLeap();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void BeginAiming();
	void CancelAiming();
	void ConfirmLeap();
	void HandleLanded();
	void ForceCancel();

	// 봇용: 커서 조준 없이 지정 지점으로 차징→자동 도약 (차징 예고는 플레이어와 동일)
	bool TryBotLeap(const FVector& TargetLocation);

	bool IsAiming() const { return Phase == ELeapPhase::Aiming; }
	bool IsLeapAttackActive() const { return Phase != ELeapPhase::None; }

	float GetMaxLeapRange() const;

private:
	void UpdateAimTarget(float ChargeAlpha);
	void UpdateBotAimTarget(float ChargeAlpha);
	bool FindGroundAlongDirection(const FVector& Origin, const FVector& FlatDirection, float MaxDistance, FVector& OutLocation) const;
	void HandleArcFinished(const FVector& ArrivalLocation);
	void RestoreCursor();
	bool IsLeapReady() const;
	const FCSKPlayerLeapSettings& GetLeapSettings() const;
	void PlayLeapStartVFX() const;

private:
	UPROPERTY()
	TObjectPtr<APlayerController> CachedPlayerController;

	ELeapPhase Phase = ELeapPhase::None;

	float NextLeapReadyTime = 0.0f;
	bool bRestoringCamera = false;

	FVector CurrentTargetLocation = FVector::ZeroVector;

	// 봇 조준 모드 (커서 대신 지정 목표로 차징)
	bool bBotDriven = false;
	FVector BotDesiredTarget = FVector::ZeroVector;

	float CurrentAimPitch = 0.0f;
	float AimYaw = 0.0f;
	float ChargeElapsed = 0.0f;

};
