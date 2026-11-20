#pragma once

#include "CoreMinimal.h"
#include "CSKSoftBodyTypes.generated.h"

UENUM(BlueprintType)
enum class ECSKSoftBodyMotionState : uint8
{
	Grounded,
	JumpAnticipation,
	Takeoff,
	Rising,
	Falling,
	Landing
};

USTRUCT(BlueprintType)
struct FCSKSoftBodyFinalAnimState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float IdleWobble = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float MoveWobble = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float MoveCompression = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float VisualHopZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float GroundAlpha = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float AirAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float BodySquash = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float BodyStretch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float JumpSquash = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float JumpStretch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float FallStretch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float LandSquash = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashImpulse = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashRollAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashRollEaseAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashSquashAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashStretchAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashRollHopZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashScaleOffsetX = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashScaleOffsetY = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashScaleOffsetZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float HitImpulse = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	FVector HitLocalDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float BodySlamImpulse = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float BodySlamSquashAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float BodySlamExpandAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float EatImpulse = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float EatOpenAlpha = 0.0f;
};

USTRUCT(BlueprintType)
struct FCSKSoftBodyAnimState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float IdleWobble = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float MoveWobble = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float MoveCompression = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float VisualHopZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float GroundAlpha = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float AirAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	ECSKSoftBodyMotionState MotionState = ECSKSoftBodyMotionState::Grounded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float BodySquash = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float BodyStretch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float JumpSquash = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float JumpStretch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float FallStretch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float LandSquash = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashImpulse = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashRollAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashRollEaseAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashSquashAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashStretchAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashRollHopZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashScaleOffsetX = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashScaleOffsetY = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float DashScaleOffsetZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float HitImpulse = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	FVector HitLocalDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float BodySlamImpulse = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float BodySlamSquashAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float BodySlamExpandAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float EatImpulse = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody")
	float EatOpenAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSK|SoftBody|Final")
	FCSKSoftBodyFinalAnimState Final;

	void UpdateFinal(float MoveWeight, float MoveCompressionWeight)
	{
		Final.IdleWobble = IdleWobble;
		Final.MoveWobble = MoveWobble * MoveWeight;
		Final.MoveCompression = MoveCompression * MoveCompressionWeight;
		Final.VisualHopZ = VisualHopZ;
		Final.GroundAlpha = GroundAlpha;
		Final.AirAlpha = AirAlpha;

		Final.BodySquash = BodySquash;
		Final.BodyStretch = BodyStretch;

		Final.JumpSquash = BodySquash;
		Final.JumpStretch = BodyStretch;
		Final.FallStretch = 0.0f;
		Final.LandSquash = 0.0f;
		Final.DashImpulse = DashImpulse;
		Final.DashRollAlpha = DashRollAlpha;
		Final.DashRollEaseAlpha = DashRollEaseAlpha;
		Final.DashSquashAlpha = DashSquashAlpha;
		Final.DashStretchAlpha = DashStretchAlpha;
		Final.DashRollHopZ = DashRollHopZ;
		Final.DashScaleOffsetX = DashScaleOffsetX;
		Final.DashScaleOffsetY = DashScaleOffsetY;
		Final.DashScaleOffsetZ = DashScaleOffsetZ;

		Final.HitImpulse = HitImpulse;
		Final.HitLocalDirection = HitLocalDirection;

		Final.BodySlamImpulse = BodySlamImpulse;
		Final.BodySlamSquashAlpha = BodySlamSquashAlpha;
		Final.BodySlamExpandAlpha = BodySlamExpandAlpha;

		Final.EatImpulse = EatImpulse;
		Final.EatOpenAlpha = EatOpenAlpha;
	}
};
