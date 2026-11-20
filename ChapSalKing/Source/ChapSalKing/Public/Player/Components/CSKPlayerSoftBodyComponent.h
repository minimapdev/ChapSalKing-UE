#pragma once

#include "CoreMinimal.h"
#include "Player/CSKSoftBodyTypes.h"
#include "Player/CSKPlayerComponentBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CSKPlayerSoftBodyComponent.generated.h"

class UDoughSoftBodyData;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKPlayerSoftBodyComponent : public UCSKPlayerComponentBase
{
	GENERATED_BODY()

public:
	UCSKPlayerSoftBodyComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void UpdateRemoteTickThrottle();

	UFUNCTION(BlueprintCallable, Category = "CSK|SoftBody")
	void NotifyJumpRequested();

	UFUNCTION(BlueprintCallable, Category = "CSK|SoftBody")
	void NotifyJumpTakeoff();

	float GetJumpAnticipationTime() const;

	UFUNCTION(BlueprintCallable, Category = "CSK|SoftBody")
	void NotifyLand(float Strength = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "CSK|SoftBody")
	void AddDashImpulse(float Strength = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "CSK|SoftBody")
	void AddHitImpulse(FVector WorldDirection, float Strength = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "CSK|SoftBody")
	void AddBodySlamImpulse(float Strength = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "CSK|SoftBody")
	void AddEatImpulse(float Strength = 1.0f);

	// Clears transient movement, hit, and action deformation before capture.
	void ResetForCapture();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|SoftBody")
	TObjectPtr<UDoughSoftBodyData> SoftBodyData;

public:
	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	FCSKSoftBodyAnimState AnimState;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float IdleWobble = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float MoveWobble = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float MoveCompression = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float VisualHopZ = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float JumpStretch = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float JumpSquash = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float FallStretch = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float LandSquash = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float DashImpulse = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float DashRollAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float DashRollEaseAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float DashSquashAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float DashStretchAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float DashRollHopZ = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float DashScaleOffsetX = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float DashScaleOffsetY = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float DashScaleOffsetZ = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float HitImpulse = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	FVector HitLocalDirection = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float GroundAlpha = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float AirAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	ECSKSoftBodyMotionState MotionState = ECSKSoftBodyMotionState::Grounded;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float BodySquash = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float BodyStretch = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float BodySlamImpulse = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float BodySlamSquashAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float BodySlamExpandAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float EatImpulse = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|SoftBody|Output")
	float EatOpenAlpha = 0.0f;

private:
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> OwnerMovement;

	TEnumAsByte<EMovementMode> PreviousMovementMode = MOVE_None;
	float MaxFallingSpeed = 0.0f;
	float StateElapsedTime = 0.0f;
	float LandImpactStrength = 1.0f;
	float IdleAlpha = 0.0f;
	float IdlePhase = 0.0f;
	float MovePhase = 0.0f;
	float VisualHopPhase = 0.0f;
	float VisualHopAlpha = 0.0f;
	float DashElapsedTime = 0.0f;
	bool bDashImpulseActive = false;
	float BodySlamElapsedTime = 0.0f;
	bool bBodySlamImpulseActive = false;
	float EatElapsedTime = 0.0f;
	bool bEatImpulseActive = false;
private:
	const UDoughSoftBodyData* GetSoftBodyData() const;
	FVector GetCurrentVelocity() const;

	void SetMotionState(ECSKSoftBodyMotionState NewState);
	void UpdateMovementEvents(const FVector& CurrentVelocity);
	void UpdateMotionState(float DeltaTime, const FVector& CurrentVelocity);
	void UpdateIdleWobble(float DeltaTime, float Speed);
	void UpdateMoveWobble(float DeltaTime, float Speed);
	void UpdateImpulseDecay(float DeltaTime);
	void UpdateDashImpulse(float DeltaTime);
	void UpdateAnimState();
	void UpdateFinalAnimState();
	void UpdateMovementAlpha(float DeltaTime);
	void UpdateVisualHop(float DeltaTime, float Speed);
	void UpdateBodySlamImpulse(float DeltaTime);
	void UpdateEatImpulse(float DeltaTime);
};
