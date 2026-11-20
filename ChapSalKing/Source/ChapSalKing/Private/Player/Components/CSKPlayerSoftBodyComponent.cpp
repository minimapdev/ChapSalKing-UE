#include "Player/Components/CSKPlayerSoftBodyComponent.h"

#include "Player/CSKPlayerCharacter.h"
#include "Dough/DoughSoftBodyData.h"
#include "GameFramework/CharacterMovementComponent.h"

UCSKPlayerSoftBodyComponent::UCSKPlayerSoftBodyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UCSKPlayerSoftBodyComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == NM_DedicatedServer)
	{
		SetComponentTickEnabled(false);
		return;
	}

	// 로비 프리뷰 스탠드(ACSKLobbyPreviewStand)처럼 폰이 아닌 액터에도 붙는다.
	// 그때는 속도가 늘 0이라 대기 흔들림만 돌고 이동·점프 계열 출력은 자연히 잠잠하다 —
	// 프리뷰가 인게임과 같은 소프트바디를 그대로 쓰게 하려면 이 자리가 열려 있어야 한다
	OwnerCSKCharacter = Cast<ACSKPlayerCharacter>(GetOwner());
	checkf(SoftBodyData, TEXT("%s의 SoftBodyData가 설정되지 않았습니다."), *GetNameSafe(GetOwner()));

	OwnerMovement = OwnerCSKCharacter ? OwnerCSKCharacter->GetCharacterMovement() : nullptr;

	PreviousMovementMode = OwnerMovement ? OwnerMovement->MovementMode.GetValue() : MOVE_None;
}

void UCSKPlayerSoftBodyComponent::UpdateRemoteTickThrottle()
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	checkf(SoftBodyData, TEXT("%s의 SoftBodyData가 설정되지 않았습니다."), *GetNameSafe(GetOwner()));
	PrimaryComponentTick.TickInterval =
		OwnerPawn->IsLocallyControlled() ? 0.0f : SoftBodyData->RemoteTickInterval;
}

void UCSKPlayerSoftBodyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const FVector CurrentVelocity = GetCurrentVelocity();
	const float Speed = CurrentVelocity.Size2D();

	UpdateMovementEvents(CurrentVelocity);
	UpdateMovementAlpha(DeltaTime);
	UpdateIdleWobble(DeltaTime, Speed);
	UpdateMoveWobble(DeltaTime, Speed);
	UpdateVisualHop(DeltaTime, Speed);
	UpdateMotionState(DeltaTime, CurrentVelocity);
	UpdateDashImpulse(DeltaTime);
	UpdateBodySlamImpulse(DeltaTime);
	UpdateEatImpulse(DeltaTime);
	UpdateImpulseDecay(DeltaTime);
	UpdateAnimState();

	PreviousMovementMode = OwnerMovement ? OwnerMovement->MovementMode.GetValue() : MOVE_None;
}

const UDoughSoftBodyData* UCSKPlayerSoftBodyComponent::GetSoftBodyData() const
{
	return SoftBodyData;
}

FVector UCSKPlayerSoftBodyComponent::GetCurrentVelocity() const
{
	if (OwnerMovement)
	{
		return OwnerMovement->Velocity;
	}

	return OwnerCSKCharacter ? OwnerCSKCharacter->GetVelocity() : FVector::ZeroVector;
}

void UCSKPlayerSoftBodyComponent::SetMotionState(ECSKSoftBodyMotionState NewState)
{
	if (MotionState == NewState)
	{
		return;
	}

	MotionState = NewState;
	StateElapsedTime = 0.0f;
}

void UCSKPlayerSoftBodyComponent::UpdateMovementEvents(const FVector& CurrentVelocity)
{
	if (!OwnerMovement)
	{
		return;
	}

	const EMovementMode CurrentMovementMode = OwnerMovement->MovementMode;
	const bool bWasFalling = PreviousMovementMode == MOVE_Falling;
	const bool bIsFalling = CurrentMovementMode == MOVE_Falling;
	const UDoughSoftBodyData* Data = GetSoftBodyData();

	if (bIsFalling)
	{
		MaxFallingSpeed = FMath::Max(MaxFallingSpeed, FMath::Max(0.0f, -CurrentVelocity.Z));

		if (!bWasFalling && CurrentVelocity.Z > Data->JumpMinUpwardVelocity)
		{
			NotifyJumpTakeoff();
		}

		return;
	}

	if (bWasFalling)
	{
		const float ReferenceSpeed = FMath::Max(Data->LandStrengthReferenceSpeed, KINDA_SMALL_NUMBER);
		const float LandStrength = FMath::Clamp(
			MaxFallingSpeed / ReferenceSpeed,
			Data->MinLandStrength,
			Data->MaxLandStrength);
		NotifyLand(LandStrength);
		MaxFallingSpeed = 0.0f;
	}
}

void UCSKPlayerSoftBodyComponent::UpdateMotionState(float DeltaTime, const FVector& CurrentVelocity)
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();
	StateElapsedTime += DeltaTime;

	// 아치 비행(도약·널뛰기·달 복귀)은 MOVE_None이지만 공중 상태로 취급 — 아니면 Takeoff에 갇힌다
	const bool bIsFalling = (OwnerMovement && OwnerMovement->MovementMode == MOVE_Falling)
		|| (OwnerCSKCharacter && OwnerCSKCharacter->IsArcFlying());
	const bool bIsGrounded = OwnerMovement && OwnerMovement->IsMovingOnGround();

	if (bIsFalling)
	{
		if (MotionState == ECSKSoftBodyMotionState::JumpAnticipation)
		{
			SetMotionState(ECSKSoftBodyMotionState::Takeoff);
		}
		else if (MotionState == ECSKSoftBodyMotionState::Takeoff)
		{
			if (StateElapsedTime >= Data->TakeoffStateDuration)
			{
				SetMotionState(CurrentVelocity.Z > 0.0f ? ECSKSoftBodyMotionState::Rising : ECSKSoftBodyMotionState::Falling);
			}
		}
		else
		{
			SetMotionState(CurrentVelocity.Z > 0.0f ? ECSKSoftBodyMotionState::Rising : ECSKSoftBodyMotionState::Falling);
		}
	}
	else if (bIsGrounded)
	{
		if (MotionState == ECSKSoftBodyMotionState::Landing)
		{
			if (StateElapsedTime >= Data->LandingStateDuration)
			{
				SetMotionState(ECSKSoftBodyMotionState::Grounded);
			}
		}
		else if (MotionState == ECSKSoftBodyMotionState::JumpAnticipation)
		{
			const float MaxAnticipationTime =
				Data->JumpAnticipationTime + Data->JumpAnticipationRecoveryGraceTime;

			if (StateElapsedTime >= MaxAnticipationTime)
			{
				SetMotionState(ECSKSoftBodyMotionState::Grounded);
			}
		}
		else
		{
			SetMotionState(ECSKSoftBodyMotionState::Grounded);
		}
	}

	float TargetSquash = 0.0f;
	float TargetStretch = 0.0f;

	switch (MotionState)
	{
	case ECSKSoftBodyMotionState::JumpAnticipation:
		TargetSquash = Data->JumpSquashAmount;
		break;

	case ECSKSoftBodyMotionState::Takeoff:
		TargetStretch = Data->JumpStretchAmount;
		break;

	case ECSKSoftBodyMotionState::Rising:
		TargetStretch = Data->JumpStretchAmount * Data->RisingStretchScale;
		break;

	case ECSKSoftBodyMotionState::Falling:
	{
		const float FallSpeed = FMath::Max(0.0f, -CurrentVelocity.Z);
		const float ReferenceSpeed = FMath::Max(Data->FallStretchReferenceSpeed, KINDA_SMALL_NUMBER);
		TargetStretch = FMath::Clamp(FallSpeed / ReferenceSpeed, 0.0f, 1.0f) * Data->FallStretchScale;
		break;
	}

	case ECSKSoftBodyMotionState::Landing:
		TargetSquash = Data->LandSquashAmount * LandImpactStrength;
		break;

	case ECSKSoftBodyMotionState::Grounded:
	default:
		TargetSquash = MoveCompression;
		break;
	}

	BodySquash = FMath::FInterpTo(BodySquash, TargetSquash, DeltaTime, Data->BodySquashInterpSpeed);
	BodyStretch = FMath::FInterpTo(BodyStretch, TargetStretch, DeltaTime, Data->BodyStretchInterpSpeed);

	JumpSquash = MotionState == ECSKSoftBodyMotionState::JumpAnticipation ? BodySquash : 0.0f;
	JumpStretch = (MotionState == ECSKSoftBodyMotionState::Takeoff || MotionState == ECSKSoftBodyMotionState::Rising) ? BodyStretch : 0.0f;
	FallStretch = MotionState == ECSKSoftBodyMotionState::Falling ? BodyStretch : 0.0f;
	LandSquash = MotionState == ECSKSoftBodyMotionState::Landing ? BodySquash : 0.0f;
}

void UCSKPlayerSoftBodyComponent::AddBodySlamImpulse(float Strength)
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();

	BodySlamImpulse = FMath::Clamp(FMath::Max(BodySlamImpulse, Strength), 0.0f, Data->MaxImpulse);
	BodySlamElapsedTime = 0.0f;
	bBodySlamImpulseActive = true;

	UpdateAnimState();
}

void UCSKPlayerSoftBodyComponent::UpdateBodySlamImpulse(float DeltaTime)
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();

	if (!bBodySlamImpulseActive)
	{
		BodySlamImpulse = FMath::FInterpTo(BodySlamImpulse, 0.0f, DeltaTime, Data->BodySlamImpulseDecaySpeed);
		BodySlamSquashAlpha = FMath::FInterpTo(BodySlamSquashAlpha, 0.0f, DeltaTime, Data->BodySlamImpulseDecaySpeed);
		BodySlamExpandAlpha = FMath::FInterpTo(BodySlamExpandAlpha, 0.0f, DeltaTime, Data->BodySlamImpulseDecaySpeed);

		if (BodySlamImpulse <= 0.01f &&
			BodySlamSquashAlpha <= 0.01f &&
			BodySlamExpandAlpha <= 0.01f)
		{
			BodySlamImpulse = 0.0f;
			BodySlamSquashAlpha = 0.0f;
			BodySlamExpandAlpha = 0.0f;
		}

		return;
	}

	BodySlamElapsedTime += DeltaTime;

	const float Duration = FMath::Max(Data->BodySlamPulseDuration, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(BodySlamElapsedTime / Duration, 0.0f, 1.0f);

	const float SquashEndAlpha = FMath::Clamp(Data->BodySlamSquashEndAlpha, KINDA_SMALL_NUMBER, 1.0f);
	const float ExpandPeakAlpha = FMath::Clamp(Data->BodySlamExpandPeakAlpha, KINDA_SMALL_NUMBER, 1.0f - KINDA_SMALL_NUMBER);

	const float SquashPhase = FMath::Clamp(Alpha / SquashEndAlpha, 0.0f, 1.0f);
	BodySlamSquashAlpha = FMath::Sin(SquashPhase * PI);

	if (Alpha <= ExpandPeakAlpha)
	{
		const float RiseAlpha = FMath::Clamp(Alpha / ExpandPeakAlpha, 0.0f, 1.0f);
		BodySlamExpandAlpha = FMath::InterpEaseOut(0.0f, 1.0f, RiseAlpha, 2.0f);
	}
	else
	{
		const float FallAlpha = FMath::Clamp((Alpha - ExpandPeakAlpha) / (1.0f - ExpandPeakAlpha), 0.0f, 1.0f);
		BodySlamExpandAlpha = FMath::InterpEaseIn(1.0f, 0.0f, FallAlpha, 2.0f);
	}

	if (Alpha >= 1.0f)
	{
		bBodySlamImpulseActive = false;
	}
}

void UCSKPlayerSoftBodyComponent::AddEatImpulse(float Strength)
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();

	EatImpulse = FMath::Clamp(FMath::Max(EatImpulse, Strength), 0.0f, Data->MaxImpulse);
	EatElapsedTime = 0.0f;
	bEatImpulseActive = true;

	UpdateAnimState();
}

void UCSKPlayerSoftBodyComponent::UpdateEatImpulse(float DeltaTime)
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();

	if (!bEatImpulseActive)
	{
		EatImpulse = FMath::FInterpTo(EatImpulse, 0.0f, DeltaTime, Data->EatImpulseDecaySpeed);
		EatOpenAlpha = FMath::FInterpTo(EatOpenAlpha, 0.0f, DeltaTime, Data->EatImpulseDecaySpeed);

		if (EatImpulse <= 0.01f && EatOpenAlpha <= 0.01f)
		{
			EatImpulse = 0.0f;
			EatOpenAlpha = 0.0f;
		}

		return;
	}

	EatElapsedTime += DeltaTime;

	const float Duration = FMath::Max(Data->EatPulseDuration, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(EatElapsedTime / Duration, 0.0f, 1.0f);
	const float PeakAlpha = FMath::Clamp(Data->EatOpenPeakAlpha, KINDA_SMALL_NUMBER, 1.0f - KINDA_SMALL_NUMBER);

	if (Alpha <= PeakAlpha)
	{
		const float RiseAlpha = FMath::Clamp(Alpha / PeakAlpha, 0.0f, 1.0f);
		EatOpenAlpha = FMath::InterpEaseOut(0.0f, 1.0f, RiseAlpha, 2.0f);
	}
	else
	{
		const float FallAlpha = FMath::Clamp((Alpha - PeakAlpha) / (1.0f - PeakAlpha), 0.0f, 1.0f);
		EatOpenAlpha = FMath::InterpEaseIn(1.0f, 0.0f, FallAlpha, 2.0f);
	}

	if (Alpha >= 1.0f)
	{
		bEatImpulseActive = false;
	}
}

void UCSKPlayerSoftBodyComponent::ResetForCapture()
{
	PreviousMovementMode = OwnerMovement ? OwnerMovement->MovementMode.GetValue() : MOVE_None;
	MaxFallingSpeed = 0.0f;
	StateElapsedTime = 0.0f;
	LandImpactStrength = 0.0f;
	IdleAlpha = 0.0f;
	IdlePhase = 0.0f;
	IdleWobble = 0.0f;
	MovePhase = 0.0f;
	MoveWobble = 0.0f;
	MoveCompression = 0.0f;
	VisualHopPhase = 0.0f;
	VisualHopAlpha = 0.0f;
	VisualHopZ = 0.0f;
	MotionState = ECSKSoftBodyMotionState::Grounded;
	GroundAlpha = 1.0f;
	AirAlpha = 0.0f;
	BodySquash = 0.0f;
	BodyStretch = 0.0f;
	JumpSquash = 0.0f;
	JumpStretch = 0.0f;
	FallStretch = 0.0f;
	LandSquash = 0.0f;
	DashElapsedTime = 0.0f;
	bDashImpulseActive = false;
	DashImpulse = 0.0f;
	DashRollAlpha = 0.0f;
	DashRollEaseAlpha = 0.0f;
	DashSquashAlpha = 0.0f;
	DashStretchAlpha = 0.0f;
	DashRollHopZ = 0.0f;
	DashScaleOffsetX = 0.0f;
	DashScaleOffsetY = 0.0f;
	DashScaleOffsetZ = 0.0f;
	HitImpulse = 0.0f;
	HitLocalDirection = FVector::ZeroVector;
	BodySlamElapsedTime = 0.0f;
	bBodySlamImpulseActive = false;
	BodySlamImpulse = 0.0f;
	BodySlamSquashAlpha = 0.0f;
	BodySlamExpandAlpha = 0.0f;
	EatElapsedTime = 0.0f;
	bEatImpulseActive = false;
	EatImpulse = 0.0f;
	EatOpenAlpha = 0.0f;
	AnimState = FCSKSoftBodyAnimState();
	UpdateAnimState();
}

void UCSKPlayerSoftBodyComponent::UpdateMovementAlpha(float DeltaTime)
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();

	const bool bIsGrounded = OwnerMovement && OwnerMovement->IsMovingOnGround();
	const float TargetGroundAlpha = bIsGrounded ? 1.0f : 0.0f;

	GroundAlpha = FMath::FInterpTo(
		GroundAlpha,
		TargetGroundAlpha,
		DeltaTime,
		Data->GroundAlphaInterpSpeed
	);

	AirAlpha = 1.0f - GroundAlpha;
}

void UCSKPlayerSoftBodyComponent::UpdateIdleWobble(float DeltaTime, float Speed)
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();

	const float TargetIdleAlpha = Speed < Data->IdleMinSpeed ? 1.0f : 0.0f;

	IdleAlpha = FMath::FInterpTo(IdleAlpha, TargetIdleAlpha, DeltaTime, Data->IdleInterpSpeed);
	IdlePhase += DeltaTime * Data->IdleWobbleFrequency;

	if (IdlePhase > UE_TWO_PI)
	{
		IdlePhase -= UE_TWO_PI;
	}

	IdleWobble = FMath::Sin(IdlePhase) * IdleAlpha * Data->IdleWobbleScale;
}

void UCSKPlayerSoftBodyComponent::UpdateMoveWobble(float DeltaTime, float Speed)
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();
	const float SpeedAlpha = FMath::Clamp(Speed / Data->MaxMoveSpeedForWobble, 0.0f, 1.0f);

	if (Speed < Data->MinMoveSpeed)
	{
		MoveWobble = FMath::FInterpTo(MoveWobble, 0.0f, DeltaTime, Data->MoveCompressionInterpSpeed);
		MoveCompression = FMath::FInterpTo(MoveCompression, 0.0f, DeltaTime, Data->MoveCompressionInterpSpeed);

		return;
	}

	MovePhase += DeltaTime * Data->MoveWobbleFrequency;

	if (MovePhase > UE_TWO_PI)
	{
		MovePhase -= UE_TWO_PI;
	}

	MoveWobble = FMath::Sin(MovePhase) * SpeedAlpha * Data->MoveWobbleScale;

	const float TargetCompression = SpeedAlpha * Data->MoveCompressionScale;

	MoveCompression = FMath::FInterpTo(MoveCompression, TargetCompression, DeltaTime, Data->MoveCompressionInterpSpeed);
}

void UCSKPlayerSoftBodyComponent::UpdateImpulseDecay(float DeltaTime)
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();

	HitImpulse = FMath::FInterpTo(HitImpulse, 0.0f, DeltaTime, Data->ImpulseDecaySpeed);

	if (HitImpulse <= 0.01f)
	{
		HitImpulse = 0.0f;
		HitLocalDirection = FVector::ZeroVector;
	}
}

void UCSKPlayerSoftBodyComponent::UpdateDashImpulse(float DeltaTime)
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();

	if (!bDashImpulseActive)
	{
		DashImpulse = FMath::FInterpTo(DashImpulse, 0.0f, DeltaTime, Data->ImpulseDecaySpeed);
		DashRollAlpha = 0.0f;
		DashRollEaseAlpha = 0.0f;
		DashSquashAlpha = 0.0f;
		DashStretchAlpha = 0.0f;
		DashRollHopZ = 0.0f;
		DashScaleOffsetX = 0.0f;
		DashScaleOffsetY = 0.0f;
		DashScaleOffsetZ = 0.0f;
		return;
	}

	DashElapsedTime += DeltaTime;

	const float RollDuration = FMath::Max(Data->DashRollDuration, KINDA_SMALL_NUMBER);
	const float RollTime = DashElapsedTime - Data->DashRollDelay;
	DashRollAlpha = FMath::Clamp(RollTime / RollDuration, 0.0f, 1.0f);

	const float RollEasePower = FMath::Max(Data->DashRollEasePower, KINDA_SMALL_NUMBER);
	DashRollEaseAlpha = FMath::Pow(DashRollAlpha, RollEasePower);

	const float StartSquashEndAlpha = FMath::Max(Data->DashStartSquashEndAlpha, KINDA_SMALL_NUMBER);
	const float EndSquashStartAlpha = FMath::Clamp(Data->DashEndSquashStartAlpha, 0.0f, 1.0f - KINDA_SMALL_NUMBER);
	const float StartSquashAlpha = FMath::Clamp(1.0f - DashRollAlpha / StartSquashEndAlpha, 0.0f, 1.0f);
	const float EndSquashAlpha = FMath::Clamp(
		(DashRollAlpha - EndSquashStartAlpha) / (1.0f - EndSquashStartAlpha),
		0.0f,
		1.0f
	);

	DashSquashAlpha = FMath::Clamp(StartSquashAlpha + EndSquashAlpha, 0.0f, 1.0f);
	DashStretchAlpha = FMath::Sin(DashRollAlpha * PI);
	DashRollHopZ = DashStretchAlpha * Data->DashRollHopHeight * DashImpulse;

	const float Squash = DashSquashAlpha * DashImpulse;
	const float Stretch = DashStretchAlpha * DashImpulse;
	DashScaleOffsetX = Squash * Data->DashSquashScaleX + Stretch * Data->DashStretchScaleX;
	DashScaleOffsetY = Squash * Data->DashSquashScaleY + Stretch * Data->DashStretchScaleY;
	DashScaleOffsetZ = Squash * Data->DashSquashScaleZ + Stretch * Data->DashStretchScaleZ;

	const float DashEndTime = Data->DashRollDelay + RollDuration;

	if (DashElapsedTime >= DashEndTime)
	{
		bDashImpulseActive = false;
		DashRollAlpha = 0.0f;
		DashRollEaseAlpha = 0.0f;
		DashSquashAlpha = 0.0f;
		DashStretchAlpha = 0.0f;
		DashRollHopZ = 0.0f;
		DashScaleOffsetX = 0.0f;
		DashScaleOffsetY = 0.0f;
		DashScaleOffsetZ = 0.0f;
	}
}

void UCSKPlayerSoftBodyComponent::UpdateVisualHop(float DeltaTime, float Speed)
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();

	const float SpeedAlpha = FMath::Clamp(Speed / Data->MaxMoveSpeedForWobble, 0.0f, 1.0f);
	const bool bIsGrounded = OwnerMovement && OwnerMovement->IsMovingOnGround();

	const float TargetHopAlpha = bIsGrounded && Speed >= Data->MinMoveSpeed
		? SpeedAlpha
		: 0.0f;

	VisualHopAlpha = FMath::FInterpTo(
		VisualHopAlpha,
		TargetHopAlpha,
		DeltaTime,
		Data->VisualHopInterpSpeed
	);

	VisualHopPhase += DeltaTime * Data->VisualHopFrequency;

	if (VisualHopPhase > UE_TWO_PI)
	{
		VisualHopPhase -= UE_TWO_PI;
	}

	const float RawWave = (FMath::Sin(VisualHopPhase) + 1.0f) * 0.5f;
	const float HopWave = FMath::Pow(RawWave, Data->VisualHopPower);

	VisualHopZ = HopWave * Data->VisualHopHeight * VisualHopAlpha;
}

void UCSKPlayerSoftBodyComponent::UpdateAnimState()
{
	AnimState.IdleWobble = IdleWobble;
	AnimState.MoveWobble = MoveWobble;
	AnimState.MoveCompression = MoveCompression;
	AnimState.GroundAlpha = GroundAlpha;
	AnimState.AirAlpha = AirAlpha;
	AnimState.MotionState = MotionState;
	AnimState.BodySquash = BodySquash;
	AnimState.BodyStretch = BodyStretch;
	AnimState.JumpSquash = JumpSquash;
	AnimState.JumpStretch = JumpStretch;
	AnimState.FallStretch = FallStretch;
	AnimState.LandSquash = LandSquash;
	AnimState.DashImpulse = DashImpulse;
	AnimState.DashRollAlpha = DashRollAlpha;
	AnimState.DashRollEaseAlpha = DashRollEaseAlpha;
	AnimState.DashSquashAlpha = DashSquashAlpha;
	AnimState.DashStretchAlpha = DashStretchAlpha;
	AnimState.DashRollHopZ = DashRollHopZ;
	AnimState.DashScaleOffsetX = DashScaleOffsetX;
	AnimState.DashScaleOffsetY = DashScaleOffsetY;
	AnimState.DashScaleOffsetZ = DashScaleOffsetZ;
	AnimState.HitImpulse = HitImpulse;
	AnimState.HitLocalDirection = HitLocalDirection;
	AnimState.VisualHopZ = VisualHopZ;
	AnimState.BodySlamImpulse = BodySlamImpulse;
	AnimState.BodySlamSquashAlpha = BodySlamSquashAlpha;
	AnimState.BodySlamExpandAlpha = BodySlamExpandAlpha;
	AnimState.EatImpulse = EatImpulse;
	AnimState.EatOpenAlpha = EatOpenAlpha;
	UpdateFinalAnimState();
}

void UCSKPlayerSoftBodyComponent::UpdateFinalAnimState()
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();

	const float MoveWeight = FMath::Lerp(
		Data->AirMoveWobbleScale,
		1.0f,
		GroundAlpha
	);

	const float MoveCompressionWeight = FMath::Lerp(
		Data->AirMoveCompressionScale,
		1.0f,
		GroundAlpha
	);

	AnimState.UpdateFinal(MoveWeight, MoveCompressionWeight);
}

void UCSKPlayerSoftBodyComponent::NotifyJumpRequested()
{
	if (!OwnerMovement || !OwnerMovement->IsMovingOnGround())
	{
		return;
	}

	LandImpactStrength = 0.0f;
	LandSquash = 0.0f;
	VisualHopZ = 0.0f;
	VisualHopAlpha = 0.0f;

	if (MotionState == ECSKSoftBodyMotionState::Landing)
	{
		BodySquash = 0.0f;
		BodyStretch = 0.0f;
	}

	SetMotionState(ECSKSoftBodyMotionState::JumpAnticipation);
	UpdateAnimState();
}
void UCSKPlayerSoftBodyComponent::NotifyJumpTakeoff()
{
	LandImpactStrength = 0.0f;
	LandSquash = 0.0f;
	VisualHopZ = 0.0f;
	VisualHopAlpha = 0.0f;

	SetMotionState(ECSKSoftBodyMotionState::Takeoff);
	UpdateAnimState();
}

float UCSKPlayerSoftBodyComponent::GetJumpAnticipationTime() const
{
	return GetSoftBodyData()->JumpAnticipationTime;
}

void UCSKPlayerSoftBodyComponent::NotifyLand(float Strength)
{
	LandImpactStrength = Strength;
	SetMotionState(ECSKSoftBodyMotionState::Landing);
	UpdateAnimState();
}

void UCSKPlayerSoftBodyComponent::AddDashImpulse(float Strength)
{
	const UDoughSoftBodyData* Data = GetSoftBodyData();

	DashImpulse = FMath::Clamp(FMath::Max(DashImpulse, Strength), 0.0f, Data->MaxImpulse);
	DashElapsedTime = 0.0f;
	bDashImpulseActive = true;
	UpdateAnimState();
}

void UCSKPlayerSoftBodyComponent::AddHitImpulse(FVector WorldDirection, float Strength)
{
	if (!OwnerCSKCharacter)
	{
		return;
	}

	const UDoughSoftBodyData* Data = GetSoftBodyData();

	HitLocalDirection = OwnerCSKCharacter->GetActorTransform().InverseTransformVectorNoScale(WorldDirection.GetSafeNormal());
	HitImpulse = FMath::Clamp(FMath::Max(HitImpulse, Strength), 0.0f, Data->MaxImpulse);
	UpdateAnimState();
}
