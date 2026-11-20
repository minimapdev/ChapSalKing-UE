#include "Props/Cauldron/Cauldron.h"

#include "Props/Cauldron/CauldronData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerArcFlightComponent.h"
#include "Camera/CSKCameraShakeStatics.h"
#include "Camera/CSKCameraShakeTags.h"
#include "Framework/CSKGameState.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"

// ---------------------------------------------------------------------------
// FLidHop
// ---------------------------------------------------------------------------

void ACauldron::FLidHop::Start(float InFromZ, float InApexZ, float InUp, float InDown)
{
	FromZ = InFromZ;
	ApexZ = InApexZ;
	UpDuration = FMath::Max(InUp, UE_SMALL_NUMBER);
	DownDuration = FMath::Max(InDown, UE_SMALL_NUMBER);
	Elapsed = 0.0f;
	bActive = true;
}

float ACauldron::FLidHop::Advance(float DeltaSeconds)
{
	if (!bActive)
	{
		return 0.0f;
	}

	Elapsed += DeltaSeconds;
	if (Elapsed >= UpDuration + DownDuration)
	{
		bActive = false;
		return 0.0f;
	}

	if (Elapsed < UpDuration)
	{
		const float Alpha = FMath::InterpEaseOut(0.0f, 1.0f, Elapsed / UpDuration, 2.0f);
		return FMath::Lerp(FromZ, ApexZ, Alpha);
	}

	const float Alpha = FMath::InterpEaseIn(0.0f, 1.0f, (Elapsed - UpDuration) / DownDuration, 2.0f);
	return FMath::Lerp(ApexZ, 0.0f, Alpha);
}

// ---------------------------------------------------------------------------
// ACauldron
// ---------------------------------------------------------------------------

ACauldron::ACauldron()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	EnterVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("EnterVolume"));
	EnterVolume->SetupAttachment(SceneRoot);
	EnterVolume->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
	EnterVolume->SetBoxExtent(FVector(80.0f, 80.0f, 60.0f));
	EnterVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EnterVolume->SetCollisionObjectType(ECC_WorldDynamic);
	EnterVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	EnterVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	LidRoot = CreateDefaultSubobject<USceneComponent>(TEXT("LidRoot"));
	LidRoot->SetupAttachment(SceneRoot);
	LidRoot->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));

	PotInterior = CreateDefaultSubobject<USceneComponent>(TEXT("PotInterior"));
	PotInterior->SetupAttachment(SceneRoot);
	PotInterior->SetRelativeLocation(FVector(0.0f, 0.0f, 30.0f));

	ExitLandingPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ExitLandingPoint"));
	ExitLandingPoint->SetupAttachment(SceneRoot);
	ExitLandingPoint->SetRelativeLocation(FVector(350.0f, 0.0f, 0.0f));

	SteamEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SteamEffect"));
	SteamEffect->SetupAttachment(SceneRoot);
	SteamEffect->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));
	SteamEffect->SetAutoActivate(false);
	SteamEffect->SetAutoDestroy(false);
}

void ACauldron::BeginPlay()
{
	Super::BeginPlay();

	EnterVolume->OnComponentBeginOverlap.AddDynamic(this, &ACauldron::OnEnterOverlap);
	LidBaseLocation = LidRoot->GetRelativeLocation();

	if (!CauldronData)
	{
		UE_LOG(LogTemp, Error, TEXT("Cauldron %s has no CauldronData."), *GetName());
		EnterVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}

	// 매치 시작 시 전부 열려 있고 김이 오른다
	Phase = ECauldronPhase::Open;
	SteamEffect->Activate();
}

void ACauldron::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ReopenTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void ACauldron::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const bool bLidLanded = TickLid(DeltaSeconds);

	// 도착 솥: 뚜껑이 먼저 솟고 LidUpEjectDelay 뒤에 반죽이 나온다
	if (Phase == ECauldronPhase::Popping && PopStep == EPopStep::LidUp
		&& LidHop.Elapsed >= CauldronData->LidUpEjectDelay)
	{
		if (IsValid(ArrivingCharacter))
		{
			Eject();
		}
		PopStep = EPopStep::LidDown;
	}

	if (bLidLanded)
	{
		UCSKCameraShakeStatics::PlayCameraShakeAtLocation(
			this, CSKCameraShakeTags::Cauldron_LidLand, LidRoot->GetComponentLocation());
		BP_OnLidLanded();

		if (Phase == ECauldronPhase::Popping)
		{
			EnterClosed();
		}
	}

	switch (Phase)
	{
	case ECauldronPhase::Sinking:
		TickSinking(DeltaSeconds);
		break;
	case ECauldronPhase::Popping:
		TickPopping(DeltaSeconds);
		break;
	default:
		break;
	}

	const bool bIdlePhase = Phase == ECauldronPhase::Open || Phase == ECauldronPhase::Closed;
	if (bIdlePhase && !LidHop.bActive)
	{
		SetActorTickEnabled(false);
	}
}

// ---------------------------------------------------------------------------
// 진입
// ---------------------------------------------------------------------------

void ACauldron::OnEnterOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	TryEnter(Cast<ACSKPlayerCharacter>(OtherActor));
}

bool ACauldron::CanEnter(const ACSKPlayerCharacter* Character) const
{
	if (!CauldronData || Phase != ECauldronPhase::Open || !Character)
	{
		return false;
	}
	return !Character->IsChapSalKing()
		&& !Character->IsSequenceLocked()
		&& !Character->IsBeingEaten();
}

void ACauldron::TryEnter(ACSKPlayerCharacter* Character)
{
	if (!CanEnter(Character))
	{
		return;
	}

	CapturedCharacter = Character;
	Character->SetSequenceLocked(true);

	Phase = ECauldronPhase::Sinking;
	PhaseElapsed = 0.0f;
	bCapturedHidden = false;

	// 솥 테두리 위로 호를 그리며 쏙. 도착 후에도 Flying 유지(bEndInFalling=false) — 숨겨진 채 떨어지지 않게
	Character->GetArcFlightComponent()->BeginArc(
		PotInterior->GetComponentLocation(),
		CauldronData->SinkDuration,
		CauldronData->SinkArcHeight,
		/*bEndInFalling*/ false,
		FCSKArcFlightFinished::CreateUObject(this, &ACauldron::HandleSunk));

	// 뚜껑이 잠시 솟았다 쿵. 반죽은 그 밑으로 들어간다
	SteamEffect->Deactivate();
	StartLidHop();

	BP_OnEntered(Character);
}

void ACauldron::TickSinking(float DeltaSeconds)
{
	if (!IsValid(CapturedCharacter) || CapturedCharacter->IsBeingEaten())
	{
		AbortSinking();
		return;
	}

	// 들어가기는 끝났고 도착 솥을 기다리는 중
	if (bCapturedHidden)
	{
		if (IsMatchFinished())
		{
			ACSKPlayerCharacter* Character = CapturedCharacter;
			CapturedCharacter = nullptr;
			bCapturedHidden = false;
			ShowCharacter(Character, CapturedPawnResponse);
			Character->SetSequenceLocked(false);
			EnterClosed();
			return;
		}
		HandOff();
	}

	// 아니면 호 비행 중. 끝나면 HandleSunk가 이어받는다
}

void ACauldron::HandleSunk(const FVector& ArrivalLocation)
{
	if (Phase == ECauldronPhase::Sinking && IsValid(CapturedCharacter))
	{
		HandOff();
	}
}

void ACauldron::HandOff()
{
	if (!bCapturedHidden)
	{
		HideCharacter(CapturedCharacter, CapturedPawnResponse);
		bCapturedHidden = true;
	}

	ACauldron* Destination = PickDestination();
	if (!Destination)
	{
		// 다른 솥이 전부 사용 중. 다음 틱에 다시
		return;
	}

	ACSKPlayerCharacter* Character = CapturedCharacter;
	CapturedCharacter = nullptr;
	bCapturedHidden = false;

	Destination->ReceiveArrival(Character, CapturedPawnResponse);
	EnterClosed();
}

void ACauldron::AbortSinking()
{
	if (IsValid(CapturedCharacter))
	{
		CapturedCharacter->GetArcFlightComponent()->Cancel();
		if (bCapturedHidden)
		{
			ShowCharacter(CapturedCharacter, CapturedPawnResponse);
		}
		CapturedCharacter->SetSequenceLocked(false);
	}
	CapturedCharacter = nullptr;
	bCapturedHidden = false;

	// 뚜껑은 솟던 대로 떨어져 닫힌다. 솥은 바로 다시 열림
	Reopen();
}

// ---------------------------------------------------------------------------
// 도착
// ---------------------------------------------------------------------------

void ACauldron::ReceiveArrival(ACSKPlayerCharacter* Character, ECollisionResponse SavedPawnResponse)
{
	GetWorldTimerManager().ClearTimer(ReopenTimerHandle);

	ArrivingCharacter = Character;
	ArrivingPawnResponse = SavedPawnResponse;

	// 숨긴 채 이 솥 안으로 옮겨 둔다. 매치가 끝나도 여기서 드러난다
	Character->SetActorLocation(
		PotInterior->GetComponentLocation(), false, nullptr, ETeleportType::TeleportPhysics);

	Phase = ECauldronPhase::Popping;
	PopStep = EPopStep::Warning;
	PhaseElapsed = 0.0f;
	SteamEffect->Deactivate();
	SetActorTickEnabled(true);

	BP_OnExitWarning();
}

void ACauldron::TickPopping(float DeltaSeconds)
{
	if (PopStep == EPopStep::LidDown)
	{
		// 착지는 Tick의 공통 처리에서 EnterClosed로 이어진다
		return;
	}

	if (!IsValid(ArrivingCharacter))
	{
		EnterClosed();
		return;
	}

	if (IsMatchFinished())
	{
		RevealArrivingImmediately();
		EnterClosed();
		return;
	}

	if (PopStep == EPopStep::Warning)
	{
		PhaseElapsed += DeltaSeconds;
		if (PhaseElapsed >= CauldronData->TransitDelay)
		{
			PopStep = EPopStep::LidUp;
			StartLidHop();
		}
	}
}

void ACauldron::Eject()
{
	ACSKPlayerCharacter* Character = ArrivingCharacter;
	ArrivingCharacter = nullptr;

	const FVector StartLocation = PotInterior->GetComponentLocation();
	const FVector LandingLocation = ExitLandingPoint->GetComponentLocation();

	Character->SetActorLocation(StartLocation, false, nullptr, ETeleportType::TeleportPhysics);
	ShowCharacter(Character, ArrivingPawnResponse);

	FVector Horiz = LandingLocation - StartLocation;
	Horiz.Z = 0.0f;
	if (!Horiz.IsNearlyZero())
	{
		Character->SetActorRotation(FRotator(0.0f, Horiz.Rotation().Yaw, 0.0f));
	}

	FlyingCharacter = Character;
	Character->GetArcFlightComponent()->BeginArc(
		LandingLocation + FVector(0.0f, 0.0f, Character->GetSimpleCollisionHalfHeight()),
		CauldronData->ExitFlightDuration,
		CauldronData->ExitArcHeight,
		/*bEndInFalling*/ true,
		FCSKArcFlightFinished::CreateUObject(this, &ACauldron::HandleLanded));

	BP_OnCharacterEjected(Character);
}

void ACauldron::HandleLanded(const FVector& ArrivalLocation)
{
	if (ACSKPlayerCharacter* Character = FlyingCharacter.Get())
	{
		Character->SetSequenceLocked(false);
	}
	FlyingCharacter = nullptr;
}

void ACauldron::RevealArrivingImmediately()
{
	ACSKPlayerCharacter* Character = ArrivingCharacter;
	ArrivingCharacter = nullptr;

	Character->SetActorLocation(
		PotInterior->GetComponentLocation() + FVector(0.0f, 0.0f, Character->GetSimpleCollisionHalfHeight()),
		false, nullptr, ETeleportType::TeleportPhysics);
	ShowCharacter(Character, ArrivingPawnResponse);
	Character->SetSequenceLocked(false);
}

// ---------------------------------------------------------------------------
// 닫힘 / 열림 — 김으로만 표시
// ---------------------------------------------------------------------------

void ACauldron::EnterClosed()
{
	Phase = ECauldronPhase::Closed;
	PhaseElapsed = 0.0f;
	ArrivingCharacter = nullptr;
	SteamEffect->Deactivate();

	if (CauldronData->ReopenDelay <= 0.0f)
	{
		Reopen();
		return;
	}

	GetWorldTimerManager().SetTimer(
		ReopenTimerHandle, this, &ACauldron::Reopen, CauldronData->ReopenDelay, false);
}

void ACauldron::Reopen()
{
	GetWorldTimerManager().ClearTimer(ReopenTimerHandle);

	Phase = ECauldronPhase::Open;
	PhaseElapsed = 0.0f;
	SteamEffect->Activate();

	BP_OnReopened();
}

// ---------------------------------------------------------------------------
// 보조
// ---------------------------------------------------------------------------

ACauldron* ACauldron::PickDestination() const
{
	TArray<ACauldron*> Candidates;
	for (TActorIterator<ACauldron> It(GetWorld()); It; ++It)
	{
		ACauldron* Other = *It;
		if (Other == this || !Other->CauldronData)
		{
			continue;
		}
		if (Other->Phase == ECauldronPhase::Sinking || Other->Phase == ECauldronPhase::Popping)
		{
			continue;
		}
		Candidates.Add(Other);
	}

	return Candidates.Num() > 0
		? Candidates[FMath::RandRange(0, Candidates.Num() - 1)]
		: nullptr;
}

void ACauldron::HideCharacter(ACSKPlayerCharacter* Character, ECollisionResponse& OutSavedResponse) const
{
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	OutSavedResponse = Capsule->GetCollisionResponseToChannel(ECC_Pawn);
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Character->SetActorHiddenInGame(true);
}

void ACauldron::ShowCharacter(ACSKPlayerCharacter* Character, ECollisionResponse SavedResponse) const
{
	Character->SetActorHiddenInGame(false);
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, SavedResponse);
}

void ACauldron::StartLidHop()
{
	// 이미 공중이면 그 높이에서 다시 솟는다
	LidHop.Start(
		LidHeight,
		CauldronData->LidHopHeight,
		CauldronData->LidHopUpDuration,
		CauldronData->LidHopDownDuration);
	SetActorTickEnabled(true);
	BP_OnLidLifted();
}

bool ACauldron::TickLid(float DeltaSeconds)
{
	if (!LidHop.bActive)
	{
		return false;
	}

	LidHeight = LidHop.Advance(DeltaSeconds);
	LidRoot->SetRelativeLocation(LidBaseLocation + FVector(0.0f, 0.0f, LidHeight));
	return !LidHop.bActive;
}

bool ACauldron::IsMatchFinished() const
{
	const UWorld* World = GetWorld();
	const ACSKGameState* GameState = World ? World->GetGameState<ACSKGameState>() : nullptr;
	return GameState && GameState->IsMatchFinished();
}
