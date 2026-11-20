#include "Props/Mortar/MortarZone.h"

#include "Camera/CSKCameraShakeStatics.h"
#include "Camera/CSKCameraShakeTags.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKChapSalKingComponent.h"
#include "Player/Components/CSKPlayerArcFlightComponent.h"
#include "Props/Mortar/PestleItem.h"
#include "Props/Mortar/PestleSpawnPoint.h"
#include "Props/Mortar/MortarZoneData.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

AMortarZone::AMortarZone()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	EnterVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("EnterVolume"));
	EnterVolume->SetupAttachment(SceneRoot);
	EnterVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EnterVolume->SetCollisionObjectType(ECC_WorldDynamic);
	EnterVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	EnterVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PestlePlantPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PestlePlantPoint"));
	PestlePlantPoint->SetupAttachment(SceneRoot);

	PestleSpinPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PestleSpinPoint"));
	PestleSpinPoint->SetupAttachment(SceneRoot);
	PestleSpinPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 500.0f));

	ArrivalPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ArrivalPoint"));
	ArrivalPoint->SetupAttachment(SceneRoot);
	ArrivalPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));

	TransformEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TransformEffect"));
	TransformEffect->SetupAttachment(SceneRoot);
	TransformEffect->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
	TransformEffect->SetAutoActivate(false);
	TransformEffect->SetAutoDestroy(false);
	TransformEffect->SetAllowScalability(false);
}

void AMortarZone::BeginPlay()
{
	Super::BeginPlay();

	EnterVolume->OnComponentBeginOverlap.AddDynamic(this, &AMortarZone::OnEnterMortar);
	TransformEffect->OnSystemFinished.AddUniqueDynamic(
		this,
		&AMortarZone::HandleTransformEffectFinished);

	if (const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this))
	{
		MortarData = Settings->MortarZoneData;
	}

	if (!MortarData)
	{
		UE_LOG(LogTemp, Error,
			TEXT("Mortar %s could not load MortarZoneData from DA_CSKSetting."),
			*GetName());
		EnterVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}

	TransformEffect->SetAsset(MortarData->TransformEffect);
}

void AMortarZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopTransformBeamShake(true);
	Super::EndPlay(EndPlayReason);
}

void AMortarZone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch (SequencePhase)
	{
	case EMortarSequencePhase::Rising:
		TickRising(DeltaSeconds);
		break;
	case EMortarSequencePhase::Spinning:
		TickSpinning(DeltaSeconds);
		break;
	case EMortarSequencePhase::Striking:
		TickStriking(DeltaSeconds);
		break;
	case EMortarSequencePhase::PlayingEffect:
		TickPlayingEffect(DeltaSeconds);
		break;
	default:
		SetActorTickEnabled(false);
		break;
	}
}

void AMortarZone::OnEnterMortar(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bAllowProximityStart)
	{
		return;
	}
	TryStartSequence(Cast<ACSKPlayerCharacter>(OtherActor));
}

void AMortarZone::NotifyPestlePickedUpInside(ACSKPlayerCharacter* Character)
{
	if (!bAllowProximityStart)
	{
		return;
	}
	TryStartSequence(Character);
}

void AMortarZone::BeginSequenceFromArrival(ACSKPlayerCharacter* Character)
{
	if (!CanStartSequence(Character))
	{
		// 도착했지만 시작 불가 — 공중에 방치하지 않도록 즉시 잠금 해제(낙하)
		if (Character)
		{
			Character->SetSequenceLocked(false);
		}
		return;
	}

	Character->SetActorLocation(
		ArrivalPoint->GetComponentLocation(), false, nullptr, ETeleportType::TeleportPhysics);
	TryStartSequence(Character);
}

FVector AMortarZone::GetArrivalLocation() const
{
	return ArrivalPoint->GetComponentLocation();
}

bool AMortarZone::CanStartSequence(const ACSKPlayerCharacter* Character) const
{
	return MortarData
		&& SequencePhase == EMortarSequencePhase::Idle
		&& CanTransformToKing(Character);
}

void AMortarZone::TryStartSequence(ACSKPlayerCharacter* Character)
{
	if (!CanStartSequence(Character))
	{
		return;
	}

	CapturedCharacter = Character;
	AnimatedPestle = Character->GetCarriedPestleItem();
	Character->SetSequenceLocked(true);
	AnimatedPestle->PrepareForMortarSequence();
	EnterVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BP_OnMortarActivated(Character);
	BeginRising();
}

void AMortarZone::BeginRising()
{
	if (!IsValid(AnimatedPestle))
	{
		AbortSequence();
		return;
	}

	PestleStartTransform = AnimatedPestle->GetActorTransform();
	PhaseElapsed = 0.0f;
	SequencePhase = EMortarSequencePhase::Rising;
	SetActorTickEnabled(true);
}

void AMortarZone::TickRising(float DeltaSeconds)
{
	if (!IsValid(AnimatedPestle) || !IsValid(CapturedCharacter))
	{
		AbortSequence();
		return;
	}

	PhaseElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(
		PhaseElapsed / FMath::Max(MortarData->RiseDuration, UE_SMALL_NUMBER),
		0.0f, 1.0f);
	const float EasedAlpha = FMath::InterpEaseOut(
		0.0f, 1.0f, Alpha, MortarData->RiseEaseExponent);
	const FVector Location = FMath::Lerp(
		PestleStartTransform.GetLocation(),
		PestleSpinPoint->GetComponentLocation(),
		EasedAlpha);
	const FQuat Rotation = FQuat::Slerp(
		PestleStartTransform.GetRotation(),
		PestlePlantPoint->GetComponentQuat(),
		EasedAlpha).GetNormalized();

	AnimatedPestle->SetActorLocationAndRotation(
		Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);

	if (Alpha >= 1.0f)
	{
		AnimatedPestle->SetActorLocationAndRotation(
			PestleSpinPoint->GetComponentLocation(),
			PestlePlantPoint->GetComponentQuat(),
			false, nullptr, ETeleportType::TeleportPhysics);
		PhaseElapsed = 0.0f;
		SequencePhase = EMortarSequencePhase::Spinning;
	}
}

void AMortarZone::TickSpinning(float DeltaSeconds)
{
	if (!IsValid(AnimatedPestle) || !IsValid(CapturedCharacter))
	{
		AbortSequence();
		return;
	}

	PhaseElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(
		PhaseElapsed / FMath::Max(MortarData->SpinDuration, UE_SMALL_NUMBER),
		0.0f, 1.0f);
	const float EasedAlpha = FMath::InterpEaseOut(
		0.0f, 1.0f, Alpha, MortarData->SpinEaseExponent);
	FRotator Rotation = PestlePlantPoint->GetComponentRotation();
	Rotation.Roll += 360.0f * FMath::Max(MortarData->SpinTurns, 1) * EasedAlpha;

	AnimatedPestle->SetActorLocationAndRotation(
		PestleSpinPoint->GetComponentLocation(),
		Rotation,
		false, nullptr, ETeleportType::TeleportPhysics);

	if (Alpha >= 1.0f)
	{
		AnimatedPestle->SetActorLocationAndRotation(
			PestleSpinPoint->GetComponentLocation(),
			PestlePlantPoint->GetComponentQuat(),
			false, nullptr, ETeleportType::TeleportPhysics);
		PhaseElapsed = 0.0f;
		SequencePhase = EMortarSequencePhase::Striking;
	}
}

void AMortarZone::TickStriking(float DeltaSeconds)
{
	if (!IsValid(AnimatedPestle) || !IsValid(CapturedCharacter))
	{
		AbortSequence();
		return;
	}

	PhaseElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(
		PhaseElapsed / FMath::Max(MortarData->StrikeDuration, UE_SMALL_NUMBER),
		0.0f, 1.0f);
	const float EasedAlpha = FMath::InterpEaseIn(
		0.0f, 1.0f, Alpha, MortarData->StrikeEaseExponent);
	const FVector Location = FMath::Lerp(
		PestleSpinPoint->GetComponentLocation(),
		PestlePlantPoint->GetComponentLocation(),
		EasedAlpha);

	AnimatedPestle->SetActorLocationAndRotation(
		Location,
		PestlePlantPoint->GetComponentQuat(),
		false, nullptr, ETeleportType::TeleportPhysics);

	if (Alpha >= 1.0f)
	{
		AnimatedPestle->PlantInMortar(PestlePlantPoint->GetComponentTransform());
		UCSKCameraShakeStatics::PlayCameraShakeAtLocation(
			this,
			CSKCameraShakeTags::Mortar_PestleImpact,
			PestlePlantPoint->GetComponentLocation());
		BP_OnPestleImpact(CapturedCharacter);
		BeginTransformEffect();
	}
}

void AMortarZone::BeginTransformEffect()
{
	PhaseElapsed = 0.0f;
	SequencePhase = EMortarSequencePhase::PlayingEffect;
	StopTransformBeamShake(true);

	if (TransformEffect && TransformEffect->GetAsset())
	{
		TransformEffect->Activate(true);
		ActiveTransformBeamShake = UCSKCameraShakeStatics::PlayCameraShakeAtLocation(
			this,
			CSKCameraShakeTags::Mortar_TransformBeam,
			TransformEffect->GetComponentLocation());
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Mortar %s has no Niagara system assigned to TransformEffect."),
			*GetName());
	}

}

void AMortarZone::TickPlayingEffect(float DeltaSeconds)
{
	if (!IsValid(CapturedCharacter))
	{
		AbortSequence();
		return;
	}

	PhaseElapsed += DeltaSeconds;
	if (PhaseElapsed >= MortarData->ReturnDelay)
	{
		LaunchReturn();
	}
}

void AMortarZone::LaunchReturn()
{
	ACSKPlayerCharacter* Character = CapturedCharacter;

	FVector ReturnLocation;
	if (!ResolveReturnLocation(ReturnLocation))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Mortar %s has no return target (PestleSpawnPoint missing)."),
			*GetName());
		FinishSequence();
		return;
	}

	const FVector Target =
		ReturnLocation + FVector(0.0f, 0.0f, Character->GetSimpleCollisionHalfHeight());

	FVector Horiz = Target - Character->GetActorLocation();
	Horiz.Z = 0.0f;
	if (!Horiz.IsNearlyZero())
	{
		Character->SetActorRotation(FRotator(0.0f, Horiz.Rotation().Yaw, 0.0f));
	}

	SequencePhase = EMortarSequencePhase::Returning;
	PhaseElapsed = 0.0f;
	SetActorTickEnabled(false);

	Character->GetArcFlightComponent()->BeginArc(
		Target,
		MortarData->ReturnFlightDuration,
		MortarData->ReturnArcHeight,
		/*bEndInFalling*/ true,
		FCSKArcFlightFinished::CreateUObject(this, &AMortarZone::HandleReturnLanded));

	BP_OnReturnLaunched(Character);
}

void AMortarZone::HandleReturnLanded(const FVector& ArrivalLocation)
{
	if (IsValid(CapturedCharacter))
	{
		// 착지하는 순간 변신 — 몸집이 커지며 바닥을 찍는다
		CapturedCharacter->TransformToChapSalKing();
		CapturedCharacter->GetChapSalKingComponent()->PlayLandingImpact();
	}
	FinishSequence();
}

bool AMortarZone::ResolveReturnLocation(FVector& OutLocation) const
{
	if (const AActor* Override = ReturnTargetOverride.Get())
	{
		OutLocation = Override->GetActorLocation();
		return true;
	}

	for (TActorIterator<APestleSpawnPoint> It(GetWorld()); It; ++It)
	{
		OutLocation = It->GetActorLocation();
		return true;
	}
	return false;
}

void AMortarZone::StopTransformBeamShake(bool bImmediately)
{
	if (!ActiveTransformBeamShake)
	{
		return;
	}

	UCSKCameraShakeStatics::StopCameraShake(
		this,
		ActiveTransformBeamShake,
		bImmediately);
	ActiveTransformBeamShake = nullptr;
}

void AMortarZone::HandleTransformEffectFinished(UNiagaraComponent* FinishedComponent)
{
	if (FinishedComponent == TransformEffect)
	{
		StopTransformBeamShake(false);
	}
}

void AMortarZone::FinishSequence()
{
	ACSKPlayerCharacter* Character = CapturedCharacter;
	if (IsValid(Character))
	{
		Character->SetSequenceLocked(false);
	}

	BP_OnMortarSequenceFinished(Character);

	SequencePhase = EMortarSequencePhase::Idle;
	PhaseElapsed = 0.0f;
	CapturedCharacter = nullptr;
	AnimatedPestle = nullptr;
	EnterVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetActorTickEnabled(false);
}

void AMortarZone::AbortSequence()
{
	if (IsValid(CapturedCharacter))
	{
		if (UCSKPlayerArcFlightComponent* ArcFlight = CapturedCharacter->GetArcFlightComponent())
		{
			ArcFlight->Cancel();
		}
		CapturedCharacter->SetSequenceLocked(false);
	}

	SequencePhase = EMortarSequencePhase::Idle;
	PhaseElapsed = 0.0f;
	CapturedCharacter = nullptr;
	AnimatedPestle = nullptr;
	EnterVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetActorTickEnabled(false);
	StopTransformBeamShake(true);
	if (TransformEffect)
	{
		TransformEffect->DeactivateImmediate();
	}
}

bool AMortarZone::CanTransformToKing(const ACSKPlayerCharacter* Character) const
{
	if (!Character)
	{
		return false;
	}

	const APestleItem* CarriedPestleItem = Character->GetCarriedPestleItem();
	return CarriedPestleItem && CarriedPestleItem->IsCarriedBy(Character);
}
