#pragma once

#include "CoreMinimal.h"
#include "Player/CSKPlayerComponentBase.h"
#include "TimerManager.h"
#include "Player/CSKPlayerStatData.h"
#include "CSKPlayerAttackComponent.generated.h"

class ACharacter;
class UInputAction;
class UInputComponent;
class UPrimitiveComponent;
class UCSKPlayerScaleComponent;
class USphereComponent;

struct FInputActionValue;

// 근접(몸통박치기/킹 포식) 전담 + 좌클릭 라우팅. 원거리는 UCSKPlayerShootComponent
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKPlayerAttackComponent : public UCSKPlayerComponentBase
{
	GENERATED_BODY()

public:
	UCSKPlayerAttackComponent();

	void InitializeAttack();
	void BindInput(UInputComponent* PlayerInputComponent);

	bool TryAttack();
	bool CanAttack() const;

private:
	void Attack(const FInputActionValue& Value);

	void BodySlamAttack();

	UFUNCTION()
	void HandleAttackHitboxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void EndBodySlam();
	void SpawnBodySlamEffect(const FCSKPlayerBodySlamVFXSettings& Settings) const;
	void SpawnBodySlamHitEffect(UPrimitiveComponent* HitComponent) const;
	void ProcessCurrentBodySlamOverlaps();
	void ProcessBodySlamHit(AActor* HitActor, UPrimitiveComponent* HitComponent);

	void CacheOwnerComponents();

	const FCSKPlayerAttackSettings& GetAttackSettings() const;
	float GetCurrentAppliedScale() const;
	float CalculateScaledValue(float BaseValue, float ScalePower) const;
	float GetScaledBodySlamKnockbackStrength(const FCSKPlayerAttackSettings& Settings) const;
	void StartAttackCooldown(float Cooldown);

private:
	UPROPERTY()
	TObjectPtr<UCSKPlayerScaleComponent> OwnerScaleComponent;

	UPROPERTY()
	TObjectPtr<USphereComponent> AttackHitboxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AttackAction;

	TSet<TWeakObjectPtr<AActor>> BodySlamHitActors;
	FTimerHandle BodySlamActiveTimerHandle;

	float BodySlamOriginalRadius = 0.0f;
	float NextAttackReadyTime = 0.0f;
	bool bBodySlamActive = false;
};
