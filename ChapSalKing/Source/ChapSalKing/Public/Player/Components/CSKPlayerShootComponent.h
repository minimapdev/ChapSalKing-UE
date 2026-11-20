#pragma once

#include "CoreMinimal.h"
#include "Player/CSKPlayerComponentBase.h"
#include "CSKPlayerShootComponent.generated.h"

class UInputAction;
class UInputComponent;
class UCSKPlayerIngredientComponent;
class UIngredientData;

struct FInputActionValue;

// 조준 상태가 바뀌거나 발사가 일어난 시점에만 브로드캐스트된다.
// 쿨다운은 연속 값이므로 위젯이 GetCooldownRatio()를 매 프레임 읽는다
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FCSKShootStateChangedSignature,
	bool, bAiming,
	float, CooldownRatio);

// 원거리(재료 슈팅) 전담: 조준 카메라 줌, 발사체/산탄/히트스캔 발사, 재료별 쿨다운.
// 우클릭 역할분기(킹=슈퍼점프 조준)도 여기서 라우팅
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKPlayerShootComponent : public UCSKPlayerComponentBase
{
	GENERATED_BODY()

public:
	UCSKPlayerShootComponent();

	void InitializeShoot();
	void BindInput(UInputComponent* PlayerInputComponent);

	bool TryShoot();

	// 봇용: 조준 상태 없이 지정 지점으로 즉시 발사 (쿨다운/재료/입력잠금은 플레이어와 공유)
	bool TryShootAt(const FVector& TargetPoint);

	void CancelAiming();

	UFUNCTION(BlueprintPure, Category = "CSK|Shoot")
	bool IsAiming() const
	{
		return bIsAiming;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Shoot")
	bool IsShootReady() const;

	// 1 = 방금 발사(쿨다운 시작), 0 = 발사 가능. 쿨다운이 0인 재료면 항상 0
	UFUNCTION(BlueprintPure, Category = "CSK|Shoot")
	float GetCooldownRatio() const;

	UPROPERTY(BlueprintAssignable, Category = "CSK|Shoot")
	FCSKShootStateChangedSignature OnShootStateChanged;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void StartAiming(const FInputActionValue& Value);
	void StopAiming(const FInputActionValue& Value);
	void ShootShot();
	bool FireShot(const FVector& AimPoint);
	FVector GetCameraAimPoint() const;
	FVector ResolveMuzzleAimPoint(const FVector& MuzzleLocation, const FVector& CameraAimPoint, float ProjectileRadius) const;
	void CacheOwnerComponents();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY()
	TObjectPtr<UCSKPlayerIngredientComponent> OwnerIngredientComponent;

	void BroadcastShootState();

	float NextShootReadyTime = 0.0f;

	// 마지막으로 적용된 쿨다운 길이 — GetCooldownRatio의 분모
	float LastCooldownDuration = 0.0f;

	bool bIsAiming = false;
};
