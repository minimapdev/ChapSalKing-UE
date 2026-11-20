#include "Player/Components/CSKPlayerLeapComponent.h"

#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKChapSalKingComponent.h"
#include "Player/Components/CSKPlayerArcFlightComponent.h"
#include "Player/Components/CSKPlayerSoftBodyComponent.h"
#include "Framework/CSKGameInstance.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"

namespace
{
	constexpr int32 GroundSearchSteps = 8;
	constexpr float GroundTraceUp = 1000.0f;
	constexpr float GroundTraceDown = 2000.0f;
	constexpr float CursorTraceDistance = 100000.0f;
}

UCSKPlayerLeapComponent::UCSKPlayerLeapComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCSKPlayerLeapComponent::InitializeLeap()
{
	CacheOwnerCSKCharacter();
	InitStatData();

	if (USpringArmComponent* Boom = OwnerCSKCharacter->GetCameraBoom())
	{
		Boom->AddTickPrerequisiteComponent(this);
	}
}

bool UCSKPlayerLeapComponent::IsLeapReady() const
{
	const UWorld* World = GetWorld();
	return World && World->GetTimeSeconds() >= NextLeapReadyTime;
}

const FCSKPlayerLeapSettings& UCSKPlayerLeapComponent::GetLeapSettings() const
{
	return PlayerStatData->LeapSettings;
}

float UCSKPlayerLeapComponent::GetMaxLeapRange() const
{
	return GetLeapSettings().MaxRange;
}

bool UCSKPlayerLeapComponent::TryBotLeap(const FVector& TargetLocation)
{
	if (Phase != ELeapPhase::None || !IsLeapReady())
	{
		return false;
	}
	if (!OwnerCSKCharacter->IsChapSalKing())
	{
		return false;
	}
	if (OwnerCSKCharacter->GetCharacterMovement()->IsFalling())
	{
		return false;
	}

	const FCSKPlayerLeapSettings& Settings = GetLeapSettings();

	Phase = ELeapPhase::Aiming;
	bBotDriven = true;
	ChargeElapsed = 0.0f;
	BotDesiredTarget = TargetLocation;
	CurrentTargetLocation = OwnerCSKCharacter->GetActorLocation();
	SetComponentTickEnabled(true);

	OwnerCSKCharacter->BP_OnLeapAimBegin(Settings.MaxRange, Settings.LandingIndicatorRadius);
	return true;
}

void UCSKPlayerLeapComponent::BeginAiming()
{
	if (Phase != ELeapPhase::None || !IsLeapReady())
	{
		return;
	}
	if (!OwnerCSKCharacter->IsChapSalKing())
	{
		return;
	}
	if (OwnerCSKCharacter->GetCharacterMovement()->IsFalling())
	{
		return;
	}

	CachedPlayerController = Cast<APlayerController>(OwnerCSKCharacter->GetController());
	if (!CachedPlayerController)
	{
		return;
	}

	const FCSKPlayerLeapSettings& Settings = GetLeapSettings();

	if (USpringArmComponent* Boom = OwnerCSKCharacter->GetCameraBoom())
	{
		const FRotator ViewRot = CachedPlayerController->GetControlRotation();
		CurrentAimPitch = FRotator::NormalizeAxis(ViewRot.Pitch);
		AimYaw = ViewRot.Yaw;
		Boom->bUsePawnControlRotation = false;
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	CachedPlayerController->SetInputMode(InputMode);
	CachedPlayerController->bShowMouseCursor = true;

	Phase = ELeapPhase::Aiming;
	bRestoringCamera = false;
	ChargeElapsed = 0.0f;
	SetComponentTickEnabled(true);

	const FVector Origin = OwnerCSKCharacter->GetActorLocation();
	FVector Forward = OwnerCSKCharacter->GetActorForwardVector();
	Forward.Z = 0.0f;
	Forward = Forward.GetSafeNormal();

	FVector InitialTarget;
	if (FindGroundAlongDirection(Origin, Forward, Settings.MinRange, InitialTarget))
	{
		CurrentTargetLocation = InitialTarget;
	}
	else
	{
		CurrentTargetLocation = Origin + Forward * Settings.MinRange;
	}

	OwnerCSKCharacter->BP_OnLeapAimBegin(Settings.MaxRange, Settings.LandingIndicatorRadius);
}

void UCSKPlayerLeapComponent::CancelAiming()
{
	if (Phase != ELeapPhase::Aiming)
	{
		return;
	}

	OwnerCSKCharacter->BP_OnLeapAimEnd();
	RestoreCursor();

	Phase = ELeapPhase::None;
	bRestoringCamera = !bBotDriven;
	bBotDriven = false;
}

void UCSKPlayerLeapComponent::ConfirmLeap()
{
	if (Phase != ELeapPhase::Aiming)
	{
		return;
	}

	const FCSKPlayerLeapSettings& Settings = GetLeapSettings();
	const FVector Target = CurrentTargetLocation;

	OwnerCSKCharacter->BP_OnLeapAimEnd();
	RestoreCursor();
	bRestoringCamera = !bBotDriven;
	bBotDriven = false;

	const float HalfHeight = OwnerCSKCharacter->GetSimpleCollisionHalfHeight();
	const FVector LeapTarget = Target + FVector(0.0f, 0.0f, HalfHeight);

	FVector Horiz = LeapTarget - OwnerCSKCharacter->GetActorLocation();
	Horiz.Z = 0.0f;
	if (!Horiz.IsNearlyZero())
	{
		OwnerCSKCharacter->SetActorRotation(FRotator(0.0f, Horiz.Rotation().Yaw, 0.0f));
	}

	PlayLeapStartVFX();

	if (UCSKPlayerSoftBodyComponent* SoftBody = OwnerCSKCharacter->GetSoftBodyComponent())
	{
		SoftBody->NotifyJumpRequested();
	}

	Phase = ELeapPhase::Leaping;
	if (const UWorld* World = GetWorld())
	{
		NextLeapReadyTime = World->GetTimeSeconds() + FMath::Max(Settings.Cooldown, 0.0f);
	}

	OwnerCSKCharacter->GetArcFlightComponent()->BeginArc(
		LeapTarget,
		Settings.LeapDuration,
		Settings.LeapArcHeight,
		/*bEndInFalling*/ true,
		FCSKArcFlightFinished::CreateUObject(this, &UCSKPlayerLeapComponent::HandleArcFinished));
}

void UCSKPlayerLeapComponent::HandleArcFinished(const FVector& ArrivalLocation)
{
	HandleLanded();
}

void UCSKPlayerLeapComponent::PlayLeapStartVFX() const
{
	if (!PlayerStatData || !OwnerCSKCharacter)
	{
		return;
	}

	const FCSKPlayerGroundVFXSettings& VFX = PlayerStatData->LeapStartVFXSettings;
	if (!VFX.System)
	{
		return;
	}

	FVector SurfaceLocation = OwnerCSKCharacter->GetActorLocation();
	FVector SurfaceNormal = FVector::UpVector;
	bool bHasSurface = false;

	const UCharacterMovementComponent* Movement = OwnerCSKCharacter->GetCharacterMovement();
	if (Movement && Movement->CurrentFloor.IsWalkableFloor())
	{
		const FHitResult& FloorHit = Movement->CurrentFloor.HitResult;
		SurfaceLocation = FloorHit.ImpactPoint;
		SurfaceNormal = FloorHit.ImpactNormal.GetSafeNormal();
		bHasSurface = !SurfaceNormal.IsNearlyZero();
	}

	if (!bHasSurface)
	{
		const UCapsuleComponent* Capsule = OwnerCSKCharacter->GetCapsuleComponent();
		const float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 0.0f;
		SurfaceLocation -= FVector::UpVector * HalfHeight;
		SurfaceNormal = FVector::UpVector;
	}

	const FRotator SurfaceRotation = FRotationMatrix::MakeFromZ(SurfaceNormal).Rotator();
	const float CharacterScale =
		FMath::Max(OwnerCSKCharacter->GetActorScale3D().GetAbsMax(), 0.01f);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		VFX.System,
		SurfaceLocation + SurfaceNormal * VFX.SurfaceOffset,
		SurfaceRotation + VFX.RotationOffset,
		FVector(CharacterScale * VFX.ScaleMultiplier));
}

void UCSKPlayerLeapComponent::HandleLanded()
{
	if (Phase != ELeapPhase::Leaping)
	{
		return;
	}

	Phase = ELeapPhase::None;

	if (!OwnerCSKCharacter->IsChapSalKing())
	{
		return;
	}

	OwnerCSKCharacter->GetChapSalKingComponent()->PlayLandingImpact();
	OwnerCSKCharacter->BP_OnLeapLanded(OwnerCSKCharacter->GetActorLocation());
}

void UCSKPlayerLeapComponent::ForceCancel()
{
	if (Phase == ELeapPhase::Aiming)
	{
		CancelAiming();
		return;
	}

	if (Phase == ELeapPhase::Leaping)
	{
		Phase = ELeapPhase::None;

		if (UCSKPlayerArcFlightComponent* ArcFlight = OwnerCSKCharacter->GetArcFlightComponent())
		{
			ArcFlight->Cancel();
		}
	}
}

void UCSKPlayerLeapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const FCSKPlayerLeapSettings& Settings = GetLeapSettings();
	USpringArmComponent* Boom = OwnerCSKCharacter->GetCameraBoom();

	if (Phase == ELeapPhase::Aiming)
	{
		ChargeElapsed += DeltaTime;

		const float RawCharge = FMath::Clamp(ChargeElapsed / Settings.ChargeDuration, 0.0f, 1.0f);
		const float ChargeAlpha = FMath::SmoothStep(0.0f, 1.0f, RawCharge);

		if (bBotDriven)
		{
			UpdateBotAimTarget(ChargeAlpha);
			if (Phase != ELeapPhase::Aiming)
			{
				return;
			}
		}
		else
		{
			UpdateAimTarget(ChargeAlpha);
		}

		FVector ToTarget = CurrentTargetLocation - OwnerCSKCharacter->GetActorLocation();
		ToTarget.Z = 0.0f;
		if (!ToTarget.IsNearlyZero())
		{
			const FRotator DesiredRotation(0.0f, ToTarget.Rotation().Yaw, 0.0f);
			const FRotator NewRotation = FMath::RInterpTo(
				OwnerCSKCharacter->GetActorRotation(), DesiredRotation, DeltaTime, Settings.AimTurnInterpSpeed);
			OwnerCSKCharacter->SetActorRotation(NewRotation);
		}

		if (Boom && !bBotDriven)
		{
			CurrentAimPitch = FMath::FInterpTo(CurrentAimPitch, Settings.AimPitch, DeltaTime, Settings.CameraRestoreSpeed);
			Boom->SetWorldRotation(FRotator(CurrentAimPitch, AimYaw, 0.0f));
			Boom->TargetArmLength = FMath::FInterpTo(
				Boom->TargetArmLength, Settings.AimArmLength, DeltaTime, Settings.CameraRestoreSpeed);
		}
		return;
	}

	if (bRestoringCamera)
	{
		if (Boom && CachedPlayerController)
		{
			const FRotator ControlRot = CachedPlayerController->GetControlRotation();
			const float ControlPitch = FRotator::NormalizeAxis(ControlRot.Pitch);
			const float CurPitch = FRotator::NormalizeAxis(Boom->GetComponentRotation().Pitch);

			const float DesiredArmLength = OwnerCSKCharacter->GetScaleDesiredCameraArmLength();

			const float NewPitch = FMath::FInterpTo(CurPitch, ControlPitch, DeltaTime, Settings.CameraRestoreSpeed);
			Boom->SetWorldRotation(FRotator(NewPitch, ControlRot.Yaw, 0.0f));
			Boom->TargetArmLength = FMath::FInterpTo(Boom->TargetArmLength, DesiredArmLength, DeltaTime, Settings.CameraRestoreSpeed);

			if (FMath::IsNearlyEqual(NewPitch, ControlPitch, 0.5f) && FMath::IsNearlyEqual(Boom->TargetArmLength, DesiredArmLength, 1.0f))
			{
				Boom->TargetArmLength = DesiredArmLength;
				Boom->bUsePawnControlRotation = true;
				bRestoringCamera = false;
			}
		}
		else
		{
			bRestoringCamera = false;
		}
	}

	if (Phase != ELeapPhase::Aiming && !bRestoringCamera)
	{
		SetComponentTickEnabled(false);
	}
}

void UCSKPlayerLeapComponent::UpdateBotAimTarget(float ChargeAlpha)
{
	const FCSKPlayerLeapSettings& Settings = GetLeapSettings();
	const float CurrentRange = FMath::Lerp(Settings.MinRange, Settings.MaxRange, ChargeAlpha);

	const FVector Origin = OwnerCSKCharacter->GetActorLocation();
	FVector Flat = BotDesiredTarget - Origin;
	Flat.Z = 0.0f;
	const float DesiredDist = Flat.Size();

	if (!Flat.IsNearlyZero())
	{
		const FVector Direction = Flat.GetSafeNormal();

		FVector GroundLocation;
		if (FindGroundAlongDirection(Origin, Direction, FMath::Min(DesiredDist, CurrentRange), GroundLocation))
		{
			CurrentTargetLocation = GroundLocation;
		}
	}

	if (UCSKGameInstance::IsDrawDebugEnabled(this))
	{
		DrawDebugCircle(
			GetWorld(),
			CurrentTargetLocation + FVector(0.0f, 0.0f, 15.0f),
			Settings.LandingIndicatorRadius, 36, FColor::Red, false, -1.0f, 0, 6.0f,
			FVector(1.0f, 0.0f, 0.0f), FVector(0.0f, 1.0f, 0.0f), false);
	}

	OwnerCSKCharacter->BP_OnLeapAimUpdate(CurrentTargetLocation, CurrentRange);

	if (CurrentRange >= DesiredDist || ChargeAlpha >= 1.0f)
	{
		if (FVector::Dist2D(CurrentTargetLocation, Origin) < Settings.MinRange)
		{
			CancelAiming();
		}
		else
		{
			ConfirmLeap();
		}
	}
}

void UCSKPlayerLeapComponent::UpdateAimTarget(float ChargeAlpha)
{
	UWorld* World = GetWorld();
	if (!CachedPlayerController || !World)
	{
		return;
	}

	const FCSKPlayerLeapSettings& Settings = GetLeapSettings();
	const float CurrentRange = FMath::Lerp(Settings.MinRange, Settings.MaxRange, ChargeAlpha);

	FVector CursorWorldOrigin;
	FVector CursorWorldDirection;
	if (!CachedPlayerController->DeprojectMousePositionToWorld(CursorWorldOrigin, CursorWorldDirection))
	{
		OwnerCSKCharacter->BP_OnLeapAimUpdate(CurrentTargetLocation, CurrentRange);
		return;
	}

	FCollisionQueryParams CursorParams;
	CursorParams.AddIgnoredActor(OwnerCSKCharacter);

	FHitResult CursorHit;
	if (World->LineTraceSingleByChannel(
			CursorHit,
			CursorWorldOrigin,
			CursorWorldOrigin + CursorWorldDirection * CursorTraceDistance,
			Settings.LeapTraceChannel,
			CursorParams))
	{
		const FVector Origin = OwnerCSKCharacter->GetActorLocation();

		FVector Flat = CursorHit.ImpactPoint - Origin;
		Flat.Z = 0.0f;
		const float Dist = Flat.Size();

		if (!Flat.IsNearlyZero())
		{
			const FVector Direction = Flat.GetSafeNormal();
			const float DesiredDistance = FMath::Min(Dist, CurrentRange);

			FVector GroundLocation;
			if (FindGroundAlongDirection(Origin, Direction, DesiredDistance, GroundLocation))
			{
				CurrentTargetLocation = GroundLocation;
			}
		}
	}

	if (UCSKGameInstance::IsDrawDebugEnabled(this))
	{
		const FVector Origin = OwnerCSKCharacter->GetActorLocation();
		const FVector GroundOffset(0.0f, 0.0f, 15.0f);

		DrawDebugCircle(
			World,
			FVector(Origin.X, Origin.Y, Origin.Z - OwnerCSKCharacter->GetSimpleCollisionHalfHeight()) + GroundOffset,
			CurrentRange, 48, FColor::Yellow, false, -1.0f, 0, 6.0f,
			FVector(1.0f, 0.0f, 0.0f), FVector(0.0f, 1.0f, 0.0f), false);

		DrawDebugCircle(
			World,
			CurrentTargetLocation + GroundOffset,
			Settings.LandingIndicatorRadius, 36, FColor::Red, false, -1.0f, 0, 6.0f,
			FVector(1.0f, 0.0f, 0.0f), FVector(0.0f, 1.0f, 0.0f), false);
	}

	OwnerCSKCharacter->BP_OnLeapAimUpdate(CurrentTargetLocation, CurrentRange);
}

bool UCSKPlayerLeapComponent::FindGroundAlongDirection(
	const FVector& Origin,
	const FVector& FlatDirection,
	float MaxDistance,
	FVector& OutLocation
) const
{
	UWorld* World = GetWorld();
	if (!World || MaxDistance <= 0.0f)
	{
		return false;
	}

	const FCSKPlayerLeapSettings& Settings = GetLeapSettings();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCSKCharacter);

	for (int32 Step = GroundSearchSteps; Step >= 1; --Step)
	{
		const float Distance = MaxDistance * Step / GroundSearchSteps;
		const FVector SampleXY = Origin + FlatDirection * Distance;

		FHitResult GroundHit;
		const FVector Start(SampleXY.X, SampleXY.Y, Origin.Z + GroundTraceUp);
		const FVector End(SampleXY.X, SampleXY.Y, Origin.Z - GroundTraceDown);

		if (World->LineTraceSingleByChannel(GroundHit, Start, End, Settings.LeapTraceChannel, Params)
			&& GroundHit.ImpactNormal.Z >= Settings.GroundNormalZThreshold)
		{
			OutLocation = GroundHit.ImpactPoint;
			return true;
		}
	}

	return false;
}

void UCSKPlayerLeapComponent::RestoreCursor()
{
	if (!CachedPlayerController)
	{
		return;
	}
	CachedPlayerController->SetInputMode(FInputModeGameOnly());
	CachedPlayerController->bShowMouseCursor = false;
}
