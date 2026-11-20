#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "DoughSoftBodyData.generated.h"

UCLASS(BlueprintType)
class CHAPSALKING_API UDoughSoftBodyData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Performance")
	float RemoteTickInterval = 0.033f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Idle")
	float IdleMinSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Idle")
	float IdleInterpSpeed = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Idle")
	float IdleWobbleFrequency = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Idle")
	float IdleWobbleScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Move")
	float MinMoveSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Move")
	float MaxMoveSpeedForWobble = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Move")
	float MoveWobbleFrequency = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Move")
	float MoveWobbleScale = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Move")
	float MoveCompressionScale = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Move")
	float MoveCompressionInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Move")
	float GroundAlphaInterpSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Hop")
	float VisualHopHeight = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Hop")
	float VisualHopFrequency = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Hop")
	float VisualHopInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Hop")
	float VisualHopPower = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Jump")
	float JumpStretchAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Jump")
	float JumpSquashAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Jump")
	float LandSquashAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Jump")
	float JumpAnticipationTime = 0.055f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Jump")
	float JumpAnticipationRecoveryGraceTime = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|State")
	float TakeoffStateDuration = 0.11f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|State")
	float LandingStateDuration = 0.09f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|State")
	float BodySquashInterpSpeed = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|State")
	float BodyStretchInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|State")
	float RisingStretchScale = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Fall")
	float FallStretchScale = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Fall")
	float FallStretchReferenceSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Jump")
	float JumpMinUpwardVelocity = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Jump")
	float LandStrengthReferenceSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Jump")
	float MinLandStrength = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Jump")
	float MaxLandStrength = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Jump")
	float MaxImpulse = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Jump")
	float ImpulseDecaySpeed = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Dash")
	float DashRollDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Dash")
	float DashRollDuration = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Dash")
	float DashRollHopHeight = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Dash")
	float DashRollEasePower = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Dash")
	float DashStartSquashEndAlpha = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Dash")
	float DashEndSquashStartAlpha = 0.72f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Dash|Scale")
	float DashSquashScaleX = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Dash|Scale")
	float DashSquashScaleY = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Dash|Scale")
	float DashSquashScaleZ = -0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Dash|Scale")
	float DashStretchScaleX = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Dash|Scale")
	float DashStretchScaleY = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Dash|Scale")
	float DashStretchScaleZ = 0.04f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Blend")
	float AirMoveWobbleScale = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Blend")
	float AirMoveCompressionScale = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|BodySlam")
	float BodySlamPulseDuration = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|BodySlam")
	float BodySlamSquashEndAlpha = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|BodySlam")
	float BodySlamExpandPeakAlpha = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|BodySlam")
	float BodySlamImpulseDecaySpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Eat")
	float EatPulseDuration = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Eat")
	float EatOpenPeakAlpha = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoftBody|Eat")
	float EatImpulseDecaySpeed = 10.0f;
};
