#include "Player/Components/CSKPlayerScaleComponent.h"

#include "Player/CSKPlayerStatData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Framework/CSKPlayerState.h"

#include "Curves/CurveFloat.h"
#include "Engine/World.h"

UCSKPlayerScaleComponent::UCSKPlayerScaleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCSKPlayerScaleComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheOwnerCSKCharacter();
	InitStatData();
	InitMass();
	ApplyScale();

	// 레벨 배치 폰은 BeginPlay 전에 possess되므로, NotifyControllerChanged 시점엔
	// 아직 질량이 0이다. 질량이 확정된 지금 한 번 더 밀어넣는다.
	// (런타임 스폰 폰은 반대 순서라 PlayerState가 아직 없고, 그쪽은 possess 때 처리된다)
	PushScoreToPlayerState();
}

void UCSKPlayerScaleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TransitionElapsedTime += DeltaTime;

	const float LinearAlpha = ActiveTransitionDuration > UE_KINDA_SMALL_NUMBER
		? FMath::Clamp(TransitionElapsedTime / ActiveTransitionDuration, 0.0f, 1.0f)
		: 1.0f;

	if (LinearAlpha >= 1.0f)
	{
		CurrentAppliedScale = TargetAppliedScale;
		ApplyCurrentScale();
		SetComponentTickEnabled(false);
		OnScaleTransitionFinished.Broadcast(CurrentAppliedScale);
		return;
	}

	const float Alpha = ActiveTransitionCurve
		? ActiveTransitionCurve->GetFloatValue(LinearAlpha)
		: FMath::InterpEaseOut(0.0f, 1.0f, LinearAlpha, 2.5f);

	CurrentAppliedScale = FMath::Lerp(StartAppliedScale, TargetAppliedScale, Alpha);
	ApplyCurrentScale();
}

float UCSKPlayerScaleComponent::GetBodyScale() const
{
	return ComputeBodyScaleFromMass();
}

float UCSKPlayerScaleComponent::GetScaleTransitionDuration() const
{
	return PlayerStatData->ScaleSettings.ScaleTransitionDuration;
}

UCurveFloat* UCSKPlayerScaleComponent::GetScaleTransitionCurve() const
{
	return PlayerStatData->ScaleSettings.ScaleTransitionCurve;
}

void UCSKPlayerScaleComponent::AddMass(float DeltaGrams)
{
	SetMass(MassGrams + DeltaGrams);
}

float UCSKPlayerScaleComponent::RemoveMass(float DeltaGrams)
{
	const float Before = MassGrams;
	SetMass(MassGrams - FMath::Max(DeltaGrams, 0.0f));
	return Before - MassGrams;
}

void UCSKPlayerScaleComponent::SetMass(float NewMassGrams)
{
	const float ClampedMass = ClampMass(NewMassGrams);

	if (FMath::IsNearlyEqual(ClampedMass, MassGrams))
	{
		return;
	}

	MassGrams = ClampedMass;
	ApplyScale();
	OnMassChanged.Broadcast(MassGrams);

	// 점수 = 몸집. 모든 질량 변경이 이 함수를 거치므로 여기서만 밀어넣는다
	PushScoreToPlayerState();
}

void UCSKPlayerScaleComponent::PushScoreToPlayerState() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	if (ACSKPlayerState* CSKPlayerState = OwnerPawn->GetPlayerState<ACSKPlayerState>())
	{
		CSKPlayerState->SetCSKScore(FMath::RoundToInt(MassGrams));
	}
}

void UCSKPlayerScaleComponent::ResetMass()
{
	SetMass(PlayerStatData->ScaleSettings.StartMassGrams);
}

void UCSKPlayerScaleComponent::SetChapSalKingScaleBonusEnabled(bool bEnabled)
{
	if (!bEnabled)
	{
		EndChapSalKingScaleBonus();
		return;
	}

	bChapSalKingScaleBonusEnabled = true;
	ApplyScale();
}

void UCSKPlayerScaleComponent::EndChapSalKingScaleBonus()
{
	if (!bChapSalKingScaleBonusEnabled)
	{
		return;
	}

	bChapSalKingScaleBonusEnabled = false;
	ApplyScale();
}

float UCSKPlayerScaleComponent::GetChapSalKingScaleBonus() const
{
	return PlayerStatData->ScaleSettings.ChapSalKingScaleBonus;
}

float UCSKPlayerScaleComponent::GetCameraDistanceScalePower() const
{
	return PlayerStatData->ScaleSettings.CameraDistanceScalePower;
}

void UCSKPlayerScaleComponent::InitMass()
{
	MassGrams = ClampMass(PlayerStatData->ScaleSettings.StartMassGrams);
	CurrentAppliedScale = CalculateAppliedScale();
}

void UCSKPlayerScaleComponent::ApplyScale()
{
	StartScaleTransition();
}

void UCSKPlayerScaleComponent::StartScaleTransition()
{
	StartAppliedScale = CurrentAppliedScale;
	TargetAppliedScale = CalculateAppliedScale();
	TransitionElapsedTime = 0.0f;
	ActiveTransitionDuration = GetScaleTransitionDuration();
	ActiveTransitionCurve = GetScaleTransitionCurve();

	if (ActiveTransitionDuration <= UE_KINDA_SMALL_NUMBER)
	{
		CurrentAppliedScale = TargetAppliedScale;
		ApplyCurrentScale();
		SetComponentTickEnabled(false);
		OnScaleTransitionFinished.Broadcast(CurrentAppliedScale);
		return;
	}

	SetComponentTickEnabled(true);
}

void UCSKPlayerScaleComponent::ApplyCurrentScale()
{
	if (OwnerCSKCharacter)
	{
		OwnerCSKCharacter->SetActorScale3D(FVector(CurrentAppliedScale));
		OwnerCSKCharacter->ApplyScaleCameraDistance(
			CurrentAppliedScale,
			GetCameraDistanceScalePower()
		);
	}

	OnScaleChanged.Broadcast(ComputeBodyScaleFromMass(), CurrentAppliedScale);
}

float UCSKPlayerScaleComponent::CalculateAppliedScale() const
{
	const float Body = ComputeBodyScaleFromMass();
	return bChapSalKingScaleBonusEnabled
		? Body + GetChapSalKingScaleBonus()
		: Body;
}

float UCSKPlayerScaleComponent::GetLossableMass() const
{
	return FMath::Max(MassGrams - PlayerStatData->ScaleSettings.MinMassGrams, 0.0f);
}

float UCSKPlayerScaleComponent::ComputeBodyScaleFromMass() const
{
	return MassGrams / PlayerStatData->ScaleSettings.GramsPerBodyScale;
}

float UCSKPlayerScaleComponent::ClampMass(float InMass) const
{
	const FCSKPlayerScaleSettings& S = PlayerStatData->ScaleSettings;
	return FMath::Clamp(InMass, S.MinMassGrams, S.MaxMassGrams);
}
