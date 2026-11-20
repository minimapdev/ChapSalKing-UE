#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/TimerHandle.h"
#include "Dough/DoughTypes.h"
#include "Combat/CSKCombatHitReceiverInterface.h"
#include "Combat/CSKCombatAgentInterface.h"
#include "Combat/CSKAffinityTypes.h"
#include "Tteok/TteokData.h"
#include "CSKPlayerCharacter.generated.h"

class UDoughData;

class UCameraComponent;
class USpringArmComponent;
class USceneComponent;
class USphereComponent;

class UCSKPlayerVisualComponent;
class UCSKPlayerStatusComponent;
class UCSKPlayerStatusVFXComponent;
class UCSKPlayerMoveComponent;
class UCSKPlayerSoftBodyComponent;
class UCSKPlayerScaleComponent;
class UCSKPlayerAttackComponent;
class UCSKPlayerShootComponent;
class UCSKChapSalKingComponent;
class UCSKEatableComponent;
class UCSKPlayerLeapComponent;
class UCSKPlayerArcFlightComponent;
class UTteokData;
class UCSKPlayerIngredientComponent;
class APestleItem;

// 연출 시퀀스(널뛰기 비행·절구 변신·복귀 비행) 진입/이탈. HUD 억제에 쓴다
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSKSequenceLockChangedSignature, bool, bLocked);

UCLASS()
class CHAPSALKING_API ACSKPlayerCharacter : public ACharacter, public ICSKCombatHitReceiverInterface, public ICSKCombatAgentInterface
{
	GENERATED_BODY()

public:
	ACSKPlayerCharacter();

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void NotifyControllerChanged() override;
	virtual void PossessedBy(AController* NewController) override;

public:
	virtual void ReceiveCombatHit_Implementation(const FCSKCombatHitContext& HitContext) override;
	virtual bool RequestCombatAction_Implementation(const FCSKCombatRequest& Request) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dough")
	EDoughCharacterType DoughType = EDoughCharacterType::White;

	UFUNCTION(BlueprintPure, Category = "Dough")
	UDoughData* GetPlayerDoughData() const
	{
		return PlayerDoughData;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|ChapSalKing")
	UCSKChapSalKingComponent* GetChapSalKingComponent() const
	{
		return ChapSalKingComponent;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Status")
	UCSKPlayerStatusComponent* GetStatusComponent() const
	{
		return StatusComponent;
	}
	UFUNCTION(BlueprintPure, Category = "CSK|VFX")
	UCSKPlayerStatusVFXComponent* GetStatusVFXComponent() const
	{
		return StatusVFXComponent;
	}


	UFUNCTION(BlueprintPure, Category = "CSK|SoftBody")
	UCSKPlayerSoftBodyComponent* GetSoftBodyComponent() const
	{
		return SoftBodyComponent;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Scale")
	UCSKPlayerScaleComponent* GetScaleComponent() const
	{
		return ScaleComponent;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Move")
	UCSKPlayerMoveComponent* GetMoveComponent() const
	{
		return MoveComponent;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Attack")
	UCSKPlayerAttackComponent* GetAttackComponent() const
	{
		return AttackComponent;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Shoot")
	UCSKPlayerShootComponent* GetShootComponent() const
	{
		return ShootComponent;
	}


	UFUNCTION(BlueprintPure, Category = "CSK|Attack")
	USphereComponent* GetAttackHitboxComponent() const
	{
		return AttackHitboxComponent;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Eat")
	UCSKEatableComponent* GetEatableComponent() const
	{
		return EatableComponent;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Eat")
	bool IsBeingEaten() const;

	// 슈퍼점프 조준/비행 중 여부 (도약 중엔 넉백 물리 면역 — 커밋된 이동기)
	bool IsLeapAttackActive() const;

	UFUNCTION(BlueprintPure, Category = "CSK|Camera")
	USpringArmComponent* GetCameraBoom() const
	{
		return CameraBoom;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Leap")
	UCSKPlayerLeapComponent* GetLeapComponent() const
	{
		return LeapComponent;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|ArcFlight")
	UCSKPlayerArcFlightComponent* GetArcFlightComponent() const
	{
		return ArcFlightComponent;
	}

	// 포물선 비행 중 여부 (도약·널뛰기·달 복귀 공통). AnimBP가 공중 포즈를 잡는 데 쓴다
	UFUNCTION(BlueprintPure, Category = "CSK|ArcFlight")
	bool IsArcFlying() const;

	UFUNCTION(BlueprintPure, Category = "CSK|Ingredient")
	UCSKPlayerIngredientComponent* GetIngredientComponent() const
	{
		return IngredientComponent;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Input")
	bool IsInputLocked() const;

	// 연출 시퀀스(널뛰기·절구·복귀 비행) 동안 입력/AI/이동 잠금. 중복 호출 안전
	void SetSequenceLocked(bool bLocked);

	UFUNCTION(BlueprintPure, Category = "CSK|Sequence")
	bool IsSequenceLocked() const
	{
		return bSequenceLocked;
	}

	UPROPERTY(BlueprintAssignable, Category = "CSK|Sequence")
	FCSKSequenceLockChangedSignature OnSequenceLockChanged;

	UFUNCTION(BlueprintPure, Category = "CSK|Item")
	USceneComponent* GetPestleAttachPoint() const
	{
		return PestleAttachPoint;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Shoot")
	USceneComponent* GetShootMuzzlePoint() const
	{
		return ShootMuzzlePoint;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Item")
	APestleItem* GetCarriedPestleItem() const
	{
		return CarriedPestleItem;
	}

	UFUNCTION(BlueprintCallable, Category = "CSK|Camera")
	void ApplyScaleCameraDistance(float AppliedScale, float DistanceScalePower);

	// 현재 몸집이 요구하는 카메라 암 길이. 암 소유권은 스케일 시스템에 있고, 일시적으로 암을 바꾼 시스템(Leap 등)은 이 값으로 복귀
	UFUNCTION(BlueprintPure, Category = "CSK|Camera")
	float GetScaleDesiredCameraArmLength() const;

	UFUNCTION(BlueprintCallable, Category = "CSK|ChapSalKing")
	void TransformToChapSalKing();

	UFUNCTION(BlueprintCallable, Category = "CSK|ChapSalKing")
	void RevertFromChapSalKing();

	UFUNCTION(BlueprintPure, Category = "CSK|ChapSalKing")
	bool IsChapSalKing() const;

	// 킹 재위 중의 떡(착지 능력 데이터). 반죽 또는 기본 찹쌀킹이면 null
	UFUNCTION(BlueprintPure, Category = "CSK|ChapSalKing")
	UTteokData* GetCurrentTteokData() const;

	void ApplyChapSalKingBody(UTteokData* TteokData);
	void RevertChapSalKingBody();

	// 계열: 킹=떡의 계열, 반죽=보유 재료의 계열. 같은 계열끼리는 상태·시야류 효과 면역
	UFUNCTION(BlueprintPure, Category = "CSK|Combat")
	ECSKAffinity GetAffinity() const;

	// 받는 넉백 저항 = 몸집^지수, 킹이면 킹 저항 계수 곱연산. 공격측은 넉백을 이 값으로 나눔
	UFUNCTION(BlueprintPure, Category = "CSK|Combat")
	float GetKnockbackResistance(float ScaleResistancePower) const;

	// 먹힘 시작 시 진행 중인 조준/도약 상태를 정리 (EatableComponent가 호출)
	void NotifyBeingEatenStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|ChapSalKing")
	void BP_OnTransformToChapSalKing();

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|ChapSalKing")
	void BP_OnRevertFromChapSalKing();

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Leap")
	void BP_OnLeapAimBegin(float MaxRange, float IndicatorRadius);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Leap")
	void BP_OnLeapAimUpdate(const FVector& TargetLocation, float CurrentRange);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Leap")
	void BP_OnLeapAimEnd();

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Leap")
	void BP_OnLeapLanded(const FVector& Location);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Shoot")
	void BP_OnAimChanged(bool bAiming);

	// 콩가루 명중 시 피격자 화면 가림 (UMG는 BP에서)
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Combat")
	void BP_OnScreenObstructed(float Duration);

	// 오색 명중 시 피격자 혼란 표시 (어지럼 UI/VFX는 BP에서)
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Combat")
	void BP_OnConfused(float Duration);

	void SetCarriedPestleItem(APestleItem* InPestleItem)
	{
		CarriedPestleItem = InPestleItem;
	}

	void ClearCarriedPestleItem(APestleItem* InPestleItem)
	{
		if (CarriedPestleItem == InPestleItem)
		{
			CarriedPestleItem = nullptr;
		}
	}

private:
	UFUNCTION()
	void HandleScaleTransitionFinished(float AppliedScale);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	float BaseCameraArmLength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Item", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> PestleAttachPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Shoot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> ShootMuzzlePoint;

	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Item")
	TObjectPtr<APestleItem> CarriedPestleItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> AttackHitboxComponent;

	UPROPERTY()
	TObjectPtr<UDoughData> PlayerDoughData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKChapSalKingComponent> ChapSalKingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKPlayerStatusComponent> StatusComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKPlayerStatusVFXComponent> StatusVFXComponent;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKPlayerVisualComponent> VisualComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKPlayerMoveComponent> MoveComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKPlayerSoftBodyComponent> SoftBodyComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKPlayerScaleComponent> ScaleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKPlayerAttackComponent> AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKPlayerShootComponent> ShootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKEatableComponent> EatableComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKPlayerLeapComponent> LeapComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKPlayerArcFlightComponent> ArcFlightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKPlayerIngredientComponent> IngredientComponent;

	bool bPendingChapSalKingTransformEvent = false;
	bool bSequenceLocked = false;

private:
	void InitDoughData();
	void InitCharacterSettings();
	void InitCameraComponents();
	void ApplyDoughData();
};
