#pragma once

#include "CoreMinimal.h"
#include "Player/CSKPlayerComponentBase.h"
#include "TimerManager.h"
#include "Player/CSKPlayerStatData.h"
#include "CSKPlayerMoveComponent.generated.h"

class ACharacter;
class UInputAction;
class UInputComponent;
class UInputMappingContext;
class UCharacterMovementComponent;
class UCSKPlayerScaleComponent;

struct FInputActionValue;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FCSKSpeedEffectsChangedSignature,
	bool, bHasSlow,
	bool, bHasHaste,
	float, EffectiveFactor);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKPlayerMoveComponent : public UCSKPlayerComponentBase
{
	GENERATED_BODY()

public:
	UCSKPlayerMoveComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitializeMove();

	void RegisterInputMappingContext();

	void BindInput(UInputComponent* PlayerInputComponent);
	bool TryDash(const FVector& WorldDirection);
	bool CanDash() const;
	bool TryJump();
	void HandleLanded(const FHitResult& Hit);

	// 키네마틱 도약: 이동 물리 정지(MOVE_None) → ArcFlight 컴포넌트가 SetActorLocation으로 아치 구동.
	// CMC가 위치를 건드리지 않으므로 Velocity를 실제 아치 속도로 덮어써 애니메이션·반짝임에 넘길 수 있다
	void BeginLeapMovement();
	void EndLeapMovement();
	void ApplySpeedBoost(float Multiplier, float DurationSeconds);

	// 지속형 이속 배율 (장판 등). 같은 키로 다시 부르면 갱신
	void SetSpeedFactor(FName SourceKey, float Factor);
	void ClearSpeedFactor(FName SourceKey);
	void GetSpeedEffectState(bool& bOutHasSlow, bool& bOutHasHaste, float& OutEffectiveFactor) const;

	UPROPERTY(BlueprintAssignable, Category = "CSK|Move")
	FCSKSpeedEffectsChangedSignature OnSpeedEffectsChanged;


private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump(const FInputActionValue& Value);
	void StopJump(const FInputActionValue& Value);
	FVector GetDashDirection(const ACharacter* Character) const;
	void Dash(const FInputActionValue& Value);
	void ExecuteJump();
	void EndSpeedBoost();
	void CacheOwnerComponents();
	void BindScaleEvents();
	void ApplyMovementStats(float BodyScale);
	void BroadcastSpeedEffects();
	float GetCurrentBodyScale() const;
	bool IsInputLocked() const;
	void UpdateWalkPuff();
	bool FindWalkPuffGround(const FVector& TraceOrigin, FHitResult& OutHit) const;
	void PlayDashVFX(const FVector& DashDirection);
	void PlayGroundVFX(
		const FCSKPlayerGroundVFXSettings& Settings,
		const FHitResult* SurfaceHit = nullptr) const;

	UFUNCTION()
	void HandleScaleChanged(float BodyScale, float AppliedScale);

private:
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> OwnerMovement;

	UPROPERTY()
	TObjectPtr<UCSKPlayerScaleComponent> OwnerScaleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Input", meta = (AllowPrivateAccess = "true"))
	int32 MappingPriority = 0;

	const FCSKPlayerMovementSettings& GetMovementSettings() const;

	FTimerHandle JumpAnticipationTimerHandle;
	FTimerHandle SpeedBoostTimerHandle;
	TMap<FName, float> SpeedFactors;
	float NextDashReadyTime = 0.0f;
	FVector LastWalkPuffLocation = FVector::ZeroVector;
	bool bHasWalkPuffLocation = false;
	bool bPendingJumpLandingVFX = false;
};
