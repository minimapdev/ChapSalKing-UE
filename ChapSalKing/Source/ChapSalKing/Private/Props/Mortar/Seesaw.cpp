#include "Props/Mortar/Seesaw.h"

#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerArcFlightComponent.h"
#include "Props/Mortar/MortarZone.h"
#include "Props/Mortar/PestleItem.h"
#include "Props/Mortar/SeesawData.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"

#include "Components/BoxComponent.h"
#include "EngineUtils.h"

ASeesaw::ASeesaw()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PlankRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PlankRoot"));
	PlankRoot->SetupAttachment(SceneRoot);
	PlankRoot->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));

	// -X가 탑승측. Pitch가 +면 -X가 내려가고 +X가 올라간다
	RidePoint = CreateDefaultSubobject<USceneComponent>(TEXT("RidePoint"));
	RidePoint->SetupAttachment(PlankRoot);
	RidePoint->SetRelativeLocation(FVector(-200.0f, 0.0f, 0.0f));

	StompPoint = CreateDefaultSubobject<USceneComponent>(TEXT("StompPoint"));
	StompPoint->SetupAttachment(PlankRoot);
	StompPoint->SetRelativeLocation(FVector(200.0f, 0.0f, 0.0f));

	// 널판과 함께 기울어지도록 PlankRoot에 부착 — 내려와 있는 발판 위에서만 탑승이 잡힌다
	BoardVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BoardVolume"));
	BoardVolume->SetupAttachment(PlankRoot);
	BoardVolume->SetRelativeLocation(FVector(-200.0f, 0.0f, 50.0f));
	BoardVolume->SetBoxExtent(FVector(100.0f, 100.0f, 60.0f));
	BoardVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoardVolume->SetCollisionObjectType(ECC_WorldDynamic);
	BoardVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoardVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ASeesaw::BeginPlay()
{
	Super::BeginPlay();

	BoardVolume->OnComponentBeginOverlap.AddDynamic(this, &ASeesaw::OnBoardOverlap);
	PlankBaseRotation = PlankRoot->GetRelativeRotation();

	if (const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this))
	{
		SeesawData = Settings->SeesawData;
	}

	if (!SeesawData)
	{
		UE_LOG(LogTemp, Error,
			TEXT("Seesaw %s could not load SeesawData from DA_CSKSetting."),
			*GetName());
		BoardVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}

	// 대기 상태: 탑승 발판이 내려와 있다
	SetPlankTilt(SeesawData->TiltAngle);
}

void ASeesaw::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch (Phase)
	{
	case ESeesawPhase::Boarding:
		TickBoarding(DeltaSeconds);
		break;
	case ESeesawPhase::Springing:
		TickSpringing(DeltaSeconds);
		break;
	case ESeesawPhase::Resetting:
		TickResetting(DeltaSeconds);
		break;
	default:
		SetActorTickEnabled(false);
		break;
	}
}

void ASeesaw::OnBoardOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	TryStartLaunch(Cast<ACSKPlayerCharacter>(OtherActor));
}

bool ASeesaw::CanStartLaunch(const ACSKPlayerCharacter* Character) const
{
	if (!SeesawData || Phase != ESeesawPhase::Idle || !Character)
	{
		return false;
	}
	// 앞선 탑승자가 아직 비행 중이면 절구가 비어 보여도 재사용 금지
	if (IsValid(FlyingCharacter))
	{
		return false;
	}
	if (Character->IsChapSalKing() || Character->IsSequenceLocked())
	{
		return false;
	}

	const APestleItem* CarriedPestleItem = Character->GetCarriedPestleItem();
	if (!CarriedPestleItem || !CarriedPestleItem->IsCarriedBy(Character))
	{
		return false;
	}

	const AMortarZone* Mortar = ResolveMortar();
	return Mortar && Mortar->IsIdle();
}

void ASeesaw::TryStartLaunch(ACSKPlayerCharacter* Character)
{
	if (!CanStartLaunch(Character))
	{
		return;
	}

	CapturedCharacter = Character;
	CapturedMortar = ResolveMortar();
	Character->SetSequenceLocked(true);
	BoardVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BoardStartLocation = Character->GetActorLocation();
	Phase = ESeesawPhase::Boarding;
	PhaseElapsed = 0.0f;
	SetActorTickEnabled(true);

	BP_OnSeesawBoarded(Character);
}

void ASeesaw::TickBoarding(float DeltaSeconds)
{
	if (!IsValid(CapturedCharacter) || !IsValid(CapturedMortar))
	{
		AbortLaunch();
		return;
	}

	PhaseElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(
		PhaseElapsed / FMath::Max(SeesawData->BoardDuration, UE_SMALL_NUMBER),
		0.0f, 1.0f);
	const float EasedAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

	// 내려와 있는 발판 한가운데로 미끄러져 자리잡는다
	const FVector RideLocation = RidePoint->GetComponentLocation()
		+ FVector(0.0f, 0.0f, CapturedCharacter->GetSimpleCollisionHalfHeight());
	CapturedCharacter->SetActorLocation(
		FMath::Lerp(BoardStartLocation, RideLocation, EasedAlpha),
		false, nullptr, ETeleportType::TeleportPhysics);

	if (Alpha >= 1.0f)
	{
		Phase = ESeesawPhase::Springing;
		PhaseElapsed = 0.0f;
		BP_OnSeesawSpring(CapturedCharacter);
	}
}

void ASeesaw::TickSpringing(float DeltaSeconds)
{
	if (!IsValid(CapturedCharacter) || !IsValid(CapturedMortar))
	{
		AbortLaunch();
		return;
	}

	PhaseElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(
		PhaseElapsed / FMath::Max(SeesawData->SpringDuration, UE_SMALL_NUMBER),
		0.0f, 1.0f);
	// 끝에서 급격히 튕기도록 가속
	const float EasedAlpha = FMath::InterpEaseIn(
		0.0f, 1.0f, Alpha, SeesawData->SpringEaseExponent);

	// StompPoint가 쿵 내려찍히고 RidePoint가 솟구친다. 탑승자는 발판에 붙어 함께 올라간다
	SetPlankTilt(FMath::Lerp(SeesawData->TiltAngle, -SeesawData->TiltAngle, EasedAlpha));
	SnapCharacterToRidePoint();

	if (Alpha >= 1.0f)
	{
		LaunchToMortar();
	}
}

void ASeesaw::LaunchToMortar()
{
	ACSKPlayerCharacter* Character = CapturedCharacter;
	AMortarZone* Mortar = CapturedMortar;
	const FVector ArrivalLocation = Mortar->GetArrivalLocation();

	FVector Horiz = ArrivalLocation - Character->GetActorLocation();
	Horiz.Z = 0.0f;
	if (!Horiz.IsNearlyZero())
	{
		Character->SetActorRotation(FRotator(0.0f, Horiz.Rotation().Yaw, 0.0f));
	}

	// 탑승자는 날아갔다 — 널판은 즉시 대기 상태로 되돌아온다
	FlyingCharacter = Character;
	FlyingMortar = Mortar;
	CapturedCharacter = nullptr;
	CapturedMortar = nullptr;
	BeginReset(-SeesawData->TiltAngle);

	Character->GetArcFlightComponent()->BeginArc(
		ArrivalLocation,
		SeesawData->FlightDuration,
		SeesawData->FlightArcHeight,
		/*bEndInFalling*/ false,
		FCSKArcFlightFinished::CreateUObject(this, &ASeesaw::HandleArrival));

	BP_OnSeesawLaunched(Character);
}

void ASeesaw::HandleArrival(const FVector& ArrivalLocation)
{
	ACSKPlayerCharacter* Character = FlyingCharacter;
	AMortarZone* Mortar = FlyingMortar;
	FlyingCharacter = nullptr;
	FlyingMortar = nullptr;

	if (IsValid(Character) && IsValid(Mortar))
	{
		Mortar->BeginSequenceFromArrival(Character);
	}
	else if (IsValid(Character))
	{
		Character->SetSequenceLocked(false);
	}
}

void ASeesaw::BeginReset(float FromTilt)
{
	ResetStartTilt = FromTilt;
	Phase = ESeesawPhase::Resetting;
	PhaseElapsed = 0.0f;
	SetActorTickEnabled(true);
}

void ASeesaw::TickResetting(float DeltaSeconds)
{
	PhaseElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(
		PhaseElapsed / FMath::Max(SeesawData->ResetDuration, UE_SMALL_NUMBER),
		0.0f, 1.0f);
	const float EasedAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

	SetPlankTilt(FMath::Lerp(ResetStartTilt, SeesawData->TiltAngle, EasedAlpha));

	if (Alpha >= 1.0f)
	{
		ReturnToIdle();
	}
}

void ASeesaw::AbortLaunch()
{
	if (IsValid(CapturedCharacter))
	{
		if (UCSKPlayerArcFlightComponent* ArcFlight = CapturedCharacter->GetArcFlightComponent())
		{
			ArcFlight->Cancel();
		}
		CapturedCharacter->SetSequenceLocked(false);
	}
	CapturedCharacter = nullptr;
	CapturedMortar = nullptr;

	// 중단 시엔 현재 기울기에서 자연스럽게 대기 상태로 복귀
	const float CurrentTilt =
		(PlankRoot->GetRelativeRotation() - PlankBaseRotation).Pitch;
	BeginReset(CurrentTilt);
}

void ASeesaw::ReturnToIdle()
{
	Phase = ESeesawPhase::Idle;
	PhaseElapsed = 0.0f;
	SetActorTickEnabled(false);
	SetPlankTilt(SeesawData->TiltAngle);
	BoardVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ASeesaw::SetPlankTilt(float PitchDegrees)
{
	PlankRoot->SetRelativeRotation(
		PlankBaseRotation + FRotator(PitchDegrees, 0.0f, 0.0f));
}

void ASeesaw::SnapCharacterToRidePoint() const
{
	CapturedCharacter->SetActorLocation(
		RidePoint->GetComponentLocation()
			+ FVector(0.0f, 0.0f, CapturedCharacter->GetSimpleCollisionHalfHeight()),
		false, nullptr, ETeleportType::TeleportPhysics);
}

AMortarZone* ASeesaw::ResolveMortar() const
{
	if (AMortarZone* Assigned = TargetMortar.Get())
	{
		return Assigned;
	}

	for (TActorIterator<AMortarZone> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}
