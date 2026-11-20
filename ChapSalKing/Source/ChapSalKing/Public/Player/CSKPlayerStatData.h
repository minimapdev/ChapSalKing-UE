#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Framework/CSKDataAsset.h"
#include "CSKPlayerStatData.generated.h"

class UCurveFloat;
class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FCSKPlayerScaleSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body Scale")
	float DefaultBodyScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass", meta = (ClampMin = "1.0", Units = "g"))
	float StartMassGrams = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass", meta = (ClampMin = "1.0", Units = "g"))
	float MinMassGrams = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass", meta = (ClampMin = "1.0", Units = "g"))
	float MaxMassGrams = 20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass", meta = (ClampMin = "1.0", Units = "g"))
	float GramsPerBodyScale = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChapSalKing")
	float ChapSalKingScaleBonus = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraDistanceScalePower = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	float ScaleTransitionDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	TObjectPtr<UCurveFloat> ScaleTransitionCurve;
};

USTRUCT(BlueprintType)
struct FCSKPlayerMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Dash")
	float DashStrength = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Dash")
	float DashCooldown = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Dash")
	bool bCanDashInAir = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Dash")
	float DashSoftBodyStrength = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Scale", meta = (ClampMin = "0.0"))
	float MoveSpeedScalePower = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Scale", meta = (ClampMin = "0.1"))
	float MinMoveSpeedMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Scale", meta = (ClampMin = "0.1"))
	float MaxMoveSpeedMultiplier = 2.0f;
};

USTRUCT(BlueprintType)
struct FCSKPlayerWalkPuffSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Puff")
	TObjectPtr<UNiagaraSystem> System;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Puff", meta = (ClampMin = "0.0"))
	float MinSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Puff", meta = (ClampMin = "1.0", Units = "cm"))
	float SpawnDistance = 95.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Puff", meta = (ClampMin = "0.0"))
	float SpawnRearOffsetRadiusMultiplier = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Puff", meta = (ClampMin = "0.1"))
	float ScaleMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FCSKPlayerDashVFXSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	TObjectPtr<UNiagaraSystem> System;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0.0", Units = "cm"))
	float RearOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (Units = "cm"))
	float ZOffset = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0.0"))
	float ScaleMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	FRotator RotationOffset = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FCSKPlayerGroundVFXSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground VFX")
	TObjectPtr<UNiagaraSystem> System;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground VFX", meta = (Units = "cm"))
	float SurfaceOffset = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground VFX", meta = (ClampMin = "0.0"))
	float ScaleMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground VFX", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CharacterScaleInfluence = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground VFX", meta = (ClampMin = "0.1"))
	float MaxEffectScale = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground VFX")
	FRotator RotationOffset = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FCSKPlayerLeapLandingVFXSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Landing")
	TObjectPtr<UNiagaraSystem> System;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Landing", meta = (Units = "cm"))
	float ZOffset = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Landing", meta = (ClampMin = "1.0", Units = "cm"))
	float ReferenceRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Landing", meta = (ClampMin = "0.0"))
	float MasterScale = 1.0f;
};

USTRUCT(BlueprintType)
struct FCSKPlayerBodySlamVFXSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body Slam")
	TObjectPtr<UNiagaraSystem> System;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body Slam")
	TObjectPtr<UNiagaraSystem> HitSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body Slam", meta = (ClampMin = "0.0"))
	float ScaleMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FCSKPlayerEatAttackSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat")
	float Cooldown = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat")
	float SweepStartOffset = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat")
	float SweepLength = 320.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat")
	float SweepRadius = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat")
	float HeightOffset = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat")
	int32 MaxTargets = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat")
	float MinForwardDot = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Scale")
	float SweepRadiusScalePower = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Scale")
	float SweepLengthScalePower = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Scale")
	float SweepOffsetScalePower = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Reaction")
	float PullDuration = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Reaction")
	float BellyHoldDuration = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Reaction")
	float EjectBackwardStrength = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Reaction")
	float EjectUpwardStrength = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Reaction")
	float RecoverMinDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Reaction")
	float RecoverMaxDuration = 0.8f;

	// 뱃속 대기 후 사출 대신 킹 몸 표면에 붙일지 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Stick")
	bool bStickToKingOnEat = true;

	// 붙을 때 소켓 안쪽으로 파묻는 깊이(곡률 불일치를 표면 아래로 숨김)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Stick", meta = (ClampMin = "0.0", Units = "cm"))
	float StickEmbedDepth = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Stick", meta = (ClampMin = "0.0", ClampMax = "180.0", Units = "deg"))
	float StickFaceRandomAngle = 30.0f;

	// --- 몸부림 탈출: 붙은 반죽이 좌우 입력을 번갈아 눌러 게이지를 채운다 ---
	// 기준 속도로 연타할 때 탈출까지 걸리는 시간. 필요 전환 횟수 = EscapeSeconds × ReferenceFlipsPerSecond
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Struggle", meta = (ClampMin = "0.1", Units = "s"))
	float StruggleEscapeSeconds = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Struggle", meta = (ClampMin = "0.1"))
	float StruggleReferenceFlipsPerSecond = 5.0f;

	// 항상 적용되는 감쇠(전환 횟수/초). 연타 중에도 빠지므로 실효 속도 = 연타 − 감쇠
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Struggle", meta = (ClampMin = "0.0"))
	float StruggleDecayFlipsPerSecond = 1.0f;

	// 세션 종료 시 못 나온 반죽의 반환 비율 = 게이지 비율^Power. 1=선형, 2=킹 몫 확대
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Struggle", meta = (ClampMin = "0.1"))
	float StruggleRefundCurvePower = 1.0f;

	// 탈출 사출: 붙어 있던 표면의 바깥 법선 방향
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Struggle")
	float StruggleEscapeOutwardStrength = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Struggle")
	float StruggleEscapeUpwardStrength = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat|Debug")
	float DebugDrawDuration = 1.0f;
};

USTRUCT(BlueprintType)
struct FCSKPlayerAttackSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float AttackCooldown = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|BodySlam")
	float BodySlamActiveDuration = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|BodySlam")
	float BodySlamRadiusMultiplier = 1.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|BodySlam")
	float BodySlamKnockbackStrength = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|BodySlam")
	float BodySlamKnockbackUpwardStrength = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Scale")
	float BodySlamKnockbackScalePower = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Scale")
	float BodySlamTargetScaleResistancePower = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|BodySlam", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BodySlamDropFraction = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Eat")
	FCSKPlayerEatAttackSettings Eat;
};

USTRUCT(BlueprintType)
struct FCSKPlayerLeapSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap")
	float MinRange = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap")
	float MaxRange = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap")
	float Cooldown = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap")
	TEnumAsByte<ECollisionChannel> LeapTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GroundNormalZThreshold = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap", meta = (ClampMin = "0.01"))
	float ChargeDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap|Camera", meta = (ClampMin = "0.0"))
	float AimArmLength = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap|Camera", meta = (ClampMin = "-89.0", ClampMax = "0.0"))
	float AimPitch = -50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap|Camera", meta = (ClampMin = "0.0"))
	float CameraRestoreSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap", meta = (ClampMin = "0.1"))
	float AimTurnInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap|Indicator", meta = (ClampMin = "0.0"))
	float LandingIndicatorRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap|Jump", meta = (ClampMin = "0.05", Units = "s"))
	float LeapDuration = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap|Jump", meta = (ClampMin = "0.0"))
	float LeapArcHeight = 900.0f;
};

USTRUCT(BlueprintType)
struct FCSKChapSalKingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChapSalKing", meta = (ClampMin = "1.0"))
	float KnockbackResistance = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChapSalKing|Eat", meta = (ClampMin = "1.0"))
	float EatSpeedBoostMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChapSalKing|Eat", meta = (ClampMin = "0.0", Units = "s"))
	float EatSpeedBoostDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChapSalKing|Eat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EatMassStealFraction = 0.3f;

	// 먹힌 반죽이 붙을 킹 몸 소켓 이름 접두사 (Stick_01, Stick_02 ...)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChapSalKing|Stick")
	FName StickSocketBaseName = TEXT("Stick_");

	// 동시에 붙을 수 있는 최대 반죽 수 = 준비된 스틱 소켓 개수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChapSalKing|Stick", meta = (ClampMin = "1"))
	int32 StickSocketCount = 8;
};

USTRUCT(BlueprintType)
struct FCSKPlayerAimSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float AimZoomFactor = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim", meta = (ClampMin = "0.1"))
	float TransitionSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim", meta = (ClampMin = "0.0"))
	float SettleTolerance = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim", meta = (ClampMin = "1.0"))
	float AimTraceDistance = 10000.0f;
};

UCLASS(BlueprintType)
class CHAPSALKING_API UCSKPlayerStatData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Scale")
	FCSKPlayerScaleSettings ScaleSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Movement")
	FCSKPlayerMovementSettings MovementSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|VFX")
	FCSKPlayerWalkPuffSettings WalkPuffSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|VFX")
	FCSKPlayerDashVFXSettings DashVFXSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|VFX")
	FCSKPlayerGroundVFXSettings JumpStartVFXSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|VFX")
	FCSKPlayerGroundVFXSettings JumpLandingVFXSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|VFX")
	FCSKPlayerGroundVFXSettings LeapStartVFXSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|VFX")
	FCSKPlayerLeapLandingVFXSettings LeapLandingVFXSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|VFX")
	FCSKPlayerBodySlamVFXSettings BodySlamVFXSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Attack")
	FCSKPlayerAttackSettings AttackSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Leap")
	FCSKPlayerLeapSettings LeapSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Aim")
	FCSKPlayerAimSettings AimSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|ChapSalKing")
	FCSKChapSalKingSettings ChapSalKingSettings;
};
