#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player/CSKPlayerStatData.h"
#include "CSKEatableComponent.generated.h"

class ACharacter;
class UCapsuleComponent;
class UPrimitiveComponent;
class UCharacterMovementComponent;
class USkeletalMeshComponent;
class UCSKEatableComponent;
class UCSKChapSalKingComponent;
class UInputAction;
class UInputComponent;
struct FInputActionValue;

UENUM(BlueprintType)
enum class ECSKEatenState : uint8
{
	None,
	PullingToMouth,
	HeldInBelly,
	EjectedRecovering,
	StuckToKing
};

USTRUCT(BlueprintType)
struct FCSKEatContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Eat")
	TObjectPtr<AActor> EaterActor;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Eat")
	FVector EatDirection = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Eat")
	FCSKPlayerEatAttackSettings EatSettings;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FCSKEatConsumedSignature,
	UCSKEatableComponent*,
	const FCSKEatContext&
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSKEatenStateChangedSignature, ECSKEatenState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSKStruggleChangedSignature, float, Ratio);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKEatableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCSKEatableComponent();

	void BindInput(UInputComponent* PlayerInputComponent);

	UFUNCTION(BlueprintCallable, Category = "CSK|Eat")
	bool TryStartEatReaction(const FCSKEatContext& Context);

	UFUNCTION(BlueprintCallable, Category = "CSK|Eat")
	void CancelEatReaction();

	UFUNCTION(BlueprintPure, Category = "CSK|Eat")
	bool CanBeEatenBy(AActor* EaterActor) const;

	UFUNCTION(BlueprintPure, Category = "CSK|Eat")
	bool IsBeingEaten() const
	{
		return EatenState != ECSKEatenState::None;
	}

	bool IsBeingEatenBy(const AActor* Eater) const
	{
		return IsBeingEaten() && CurrentContext.EaterActor == Eater;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Eat")
	ECSKEatenState GetEatenState() const
	{
		return EatenState;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Eat")
	bool IsStuckToKing() const
	{
		return EatenState == ECSKEatenState::StuckToKing;
	}

	// 먹힐 때 킹에게 뜯긴 몸집. 탈출·세션 종료 시 반환 기준 (킹 컴포넌트가 기록)
	void SetStolenMass(float Grams)
	{
		StolenMassGrams = Grams;
	}

	// 몸부림 게이지 0~1
	UFUNCTION(BlueprintPure, Category = "CSK|Eat|Struggle")
	float GetStruggleRatio() const;

	// 킹 세션 종료로 풀릴 때: 게이지 비율만큼 반환하고 해제. 킹 컴포넌트가 호출
	void ReleaseFromKingSession();

	FCSKEatConsumedSignature OnEatConsumed;

	UPROPERTY(BlueprintAssignable, Category = "CSK|Eat")
	FCSKEatenStateChangedSignature OnEatenStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "CSK|Eat|Struggle")
	FCSKStruggleChangedSignature OnStruggleChanged;

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Eat")
	void BP_OnEatRejected(const FCSKEatContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Eat")
	void BP_OnEatPullStarted(const FCSKEatContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Eat")
	void BP_OnEatEjected(const FCSKEatContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Eat")
	void BP_OnEatStuck(const FCSKEatContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Eat")
	void BP_OnEatConsumed(const FCSKEatContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Eat")
	void BP_OnEatFinished(const FCSKEatContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Eat|Struggle")
	void BP_OnEscaped(const FCSKEatContext& Context);

private:
	void SetEatenState(ECSKEatenState NewState);
	void UpdateEatReaction(float DeltaTime);
	void UpdatePullingToMouth(float DeltaTime);
	void UpdateHeldInBelly(float DeltaTime);
	void UpdateEjectedRecovering(float DeltaTime);
	void UpdateStuckToKing(float DeltaTime);

	void HandleStruggleInput(const FInputActionValue& Value);
	// 좌우 전환 1회. 멀티 전환 시 서버 권위 진입점
	void AddStruggleFlips(float Flips);
	float GetRequiredStruggleFlips() const;
	void SetStruggleProgress(float NewProgress);
	// 뜯긴 몸집 × Fraction 을 킹에게서 되찾는다. 킹이 그 사이 잃었으면 남은 만큼만
	void RefundStolenMass(float Fraction);
	void EscapeFromKing();

	void LockOwnerControl();
	void UnlockOwnerControl();
	void DisableOwnerEatCollision();
	void RestoreOwnerEatCollision();
	void HideOwnerInBelly();
	void RestoreOwnerBellyHiddenState();
	void EjectOwner();
	void AttachToKingSurface();
	void DetachFromKing();
	void FinishEatReaction();

	FVector GetEatTargetLocation() const;
	ACharacter* GetOwnerCharacter() const;
	UCapsuleComponent* GetOwnerCapsule() const;
	UCharacterMovementComponent* GetOwnerMovement() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Eat", meta = (AllowPrivateAccess = "true"))
	bool bCanBeEaten = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Eat", meta = (AllowPrivateAccess = "true"))
	bool bLockCharacterControl = true;

	// 몸부림 입력. 이동 액션(IA_Move)을 그대로 지정 — X축 부호 전환만 센다
	UPROPERTY(EditDefaultsOnly, Category = "CSK|Eat|Struggle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> StruggleAction;

	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

	UPROPERTY()
	FCSKEatContext CurrentContext;

	FVector EatPullStartLocation = FVector::ZeroVector;
	float EatenStateElapsedTime = 0.0f;
	ECSKEatenState EatenState = ECSKEatenState::None;
	ECollisionResponse SavedPawnCollisionResponse = ECR_Block;
	TArray<TWeakObjectPtr<UPrimitiveComponent>> CameraCollisionComponents;
	TArray<ECollisionResponse> SavedCameraCollisionResponses;
	bool bOwnerEatCollisionDisabled = false;
	bool bSavedOwnerHiddenInGame = false;
	bool bOwnerHiddenInBelly = false;

	// 킹 표면 부착 상태
	TWeakObjectPtr<UCSKChapSalKingComponent> AttachedKingComponent;
	FName AttachedStickSocket = NAME_None;
	FTransform SavedMeshRelativeTransform = FTransform::Identity;
	bool bAttachedToKing = false;

	// 몸부림 상태
	float StruggleProgress = 0.0f;
	float StolenMassGrams = 0.0f;
	int32 LastStruggleSign = 0;
};
