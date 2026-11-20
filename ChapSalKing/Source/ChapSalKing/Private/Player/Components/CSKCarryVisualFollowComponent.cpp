#include "Player/Components/CSKCarryVisualFollowComponent.h"

#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerScaleComponent.h"
#include "Player/Components/CSKPlayerSoftBodyComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"

UCSKCarryVisualFollowComponent::UCSKCarryVisualFollowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCSKCarryVisualFollowComponent::StartFollow(
	ACSKPlayerCharacter* InCarrierCharacter,
	USceneComponent* InVisualComponent,
	const FCSKCarryVisualFollowSettings& InSettings)
{
	if (CarrierCharacter)
	{
		StopFollow();
	}

	CarrierCharacter = InCarrierCharacter;
	VisualComponent = InVisualComponent;
	Settings = InSettings;
	VisualBaseRotation = VisualComponent ? VisualComponent->GetRelativeRotation() : FRotator::ZeroRotator;

	ensureMsgf(
		!CarrierCharacter || !CarrierCharacter->GetMesh()
			|| CarrierCharacter->GetMesh()->DoesSocketExist(Settings.FollowSocketName),
		TEXT("운반자 메시에 소켓 '%s'가 없습니다 — 액터 원점으로 대체됩니다."),
		*Settings.FollowSocketName.ToString());
	PitchWobblePhase = 0.0f;
	YawWobblePhase = 0.0f;
	RollWobblePhase = 0.0f;
	MoveAlpha = 0.0f;
	CurrentJumpLiftZ = 0.0f;
	CurrentActionLiftZ = 0.0f;

	SetComponentTickEnabled(CarrierCharacter != nullptr);
}

void UCSKCarryVisualFollowComponent::StopFollow()
{
	SetComponentTickEnabled(false);

	if (VisualComponent)
	{
		VisualComponent->SetRelativeRotation(VisualBaseRotation);
	}

	CarrierCharacter = nullptr;
	VisualComponent = nullptr;
	PitchWobblePhase = 0.0f;
	YawWobblePhase = 0.0f;
	RollWobblePhase = 0.0f;
	MoveAlpha = 0.0f;
	CurrentJumpLiftZ = 0.0f;
	CurrentActionLiftZ = 0.0f;
}

void UCSKCarryVisualFollowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CarrierCharacter || !GetOwner())
	{
		StopFollow();
		return;
	}

	UpdateFollow(DeltaTime);
	UpdateVisualWobble(DeltaTime);
}

void UCSKCarryVisualFollowComponent::UpdateFollow(float DeltaTime)
{
	USkeletalMeshComponent* CharacterMesh = CarrierCharacter->GetMesh();
	if (!CharacterMesh)
	{
		return;
	}

	const FVector SocketLocation = CharacterMesh->DoesSocketExist(Settings.FollowSocketName)
		? CharacterMesh->GetSocketLocation(Settings.FollowSocketName)
		: CarrierCharacter->GetActorLocation();

	const UCSKPlayerScaleComponent* ScaleComponent = CarrierCharacter->GetScaleComponent();
	const float CarrierScale = ScaleComponent ? ScaleComponent->GetAppliedScale() : 1.0f;
	const FVector ScaledFollowOffset = Settings.FollowOffset * CarrierScale;

	const float VelocityZ = CarrierCharacter->GetVelocity().Z;
	const bool bShouldLiftForJump = VelocityZ > Settings.JumpLiftMinVelocityZ;
	const float TargetJumpLiftZ = bShouldLiftForJump? Settings.JumpLiftOffsetZ * CarrierScale: 0.0f;
	const float JumpLiftInterpSpeed = bShouldLiftForJump? Settings.JumpLiftUpInterpSpeed: Settings.JumpLiftDownInterpSpeed;

	CurrentJumpLiftZ = FMath::FInterpTo(
		CurrentJumpLiftZ,
		TargetJumpLiftZ,
		DeltaTime,
		JumpLiftInterpSpeed
	);

	const float TargetActionLiftZ = CalculateTargetActionLiftZ(CarrierScale);
	CurrentActionLiftZ = FMath::FInterpTo(
		CurrentActionLiftZ,
		TargetActionLiftZ,
		DeltaTime,
		Settings.ActionLiftInterpSpeed
	);

	const FVector TargetLocation =
		SocketLocation
		+ CarrierCharacter->GetActorForwardVector() * ScaledFollowOffset.X
		+ CarrierCharacter->GetActorRightVector() * ScaledFollowOffset.Y
		+ CarrierCharacter->GetActorUpVector() * (ScaledFollowOffset.Z + CurrentJumpLiftZ + CurrentActionLiftZ);

	AActor* Owner = GetOwner();
	FVector NewLocation = FMath::VInterpTo(
		Owner->GetActorLocation(),
		TargetLocation,
		DeltaTime,
		Settings.FollowInterpSpeed
	);

	// 고속 비행 중엔 보간이 따라오지 못해 운반물만 떨어져 남는다 — 뒤처짐을 상한으로 잡는다
	const float MaxLag = Settings.MaxFollowLag * CarrierScale;
	const FVector LagOffset = NewLocation - TargetLocation;
	if (LagOffset.SizeSquared() > FMath::Square(MaxLag))
	{
		NewLocation = TargetLocation + LagOffset.GetSafeNormal() * MaxLag;
	}

	Owner->SetActorLocation(NewLocation);

	const FRotator TargetRotation = FRotator(
		0.0f,
		CarrierCharacter->GetActorRotation().Yaw,
		0.0f
	);

	const FRotator NewRotation = FMath::RInterpTo(
		Owner->GetActorRotation(),
		TargetRotation,
		DeltaTime,
		Settings.RotationInterpSpeed
	);

	Owner->SetActorRotation(NewRotation);
}

void UCSKCarryVisualFollowComponent::UpdateVisualWobble(float DeltaTime)
{
	if (!VisualComponent)
	{
		return;
	}

	const FVector Velocity = CarrierCharacter->GetVelocity();
	const float HorizontalSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
	const float TargetMoveAlpha = FMath::Clamp(
		HorizontalSpeed / Settings.MoveSpeedForMaxWobble,
		0.0f,
		1.0f
	);

	MoveAlpha = FMath::FInterpTo(
		MoveAlpha,
		TargetMoveAlpha,
		DeltaTime,
		Settings.MoveAlphaInterpSpeed
	);

	FRotator WobbleRotation = VisualBaseRotation;

	if (Settings.bEnablePitchWobble)
	{
		const float PitchWobbleSpeed =
			Settings.PitchWobbleSpeed
			+ MoveAlpha * Settings.MoveWobbleSpeedAdd;
		const float PitchWobbleAmount =
			Settings.PitchWobbleAmount
			+ MoveAlpha * Settings.MoveWobbleAmountAdd;

		PitchWobblePhase += DeltaTime * PitchWobbleSpeed;
		WobbleRotation.Pitch += FMath::Sin(PitchWobblePhase) * PitchWobbleAmount;
	}

	if (Settings.bEnableYawWobble)
	{
		YawWobblePhase += DeltaTime * Settings.YawWobbleSpeed;
		WobbleRotation.Yaw += FMath::Sin(YawWobblePhase) * Settings.YawWobbleAmount;
	}

	if (Settings.bEnableRollWobble)
	{
		RollWobblePhase += DeltaTime * Settings.RollWobbleSpeed;
		WobbleRotation.Roll += FMath::Sin(RollWobblePhase) * Settings.RollWobbleAmount;
	}

	VisualComponent->SetRelativeRotation(WobbleRotation);
}

float UCSKCarryVisualFollowComponent::CalculateTargetActionLiftZ(float CarrierScale) const
{
	const UCSKPlayerSoftBodyComponent* SoftBody = CarrierCharacter
		? CarrierCharacter->GetSoftBodyComponent()
		: nullptr;

	if (!SoftBody)
	{
		return 0.0f;
	}

	const float DashLift = SoftBody->DashImpulse * Settings.DashLiftOffsetZ;
	const float BodySlamLift = SoftBody->BodySlamImpulse * Settings.BodySlamLiftOffsetZ;
	const float EatLift = SoftBody->EatImpulse * SoftBody->EatOpenAlpha * Settings.EatLiftOffsetZ;

	return (DashLift + BodySlamLift + EatLift) * CarrierScale;
}

