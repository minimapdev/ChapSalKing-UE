#include "Props/Snake/Snake.h"

#include "Framework/CSKGameState.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerScaleComponent.h"
#include "Props/Mortar/PestleItem.h"
#include "Dough/DoughBlob.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Player/Components/CSKPlayerSoftBodyComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "LandscapeProxy.h"
#include "DrawDebugHelpers.h"

ASnake::ASnake()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(SceneRoot);
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MouthVolume = CreateDefaultSubobject<USphereComponent>(TEXT("MouthVolume"));
	// HeadMesh가 아니라 루트에 붙인다. HeadMesh 밑에 두면 HeadMeshScale이 자식에게
	// 그대로 곱해져서 반지름이 월드 단위가 아니게 된다 — 눈으로 맞출 수 없는 값이 된다
	MouthVolume->SetupAttachment(SceneRoot);
	MouthVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MouthVolume->SetCollisionObjectType(ECC_WorldDynamic);
	MouthVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	// 막지 않는다. 삼켜지려면 겹쳐야 한다 (사양 5.3).
	// 실제 판정은 UpdateBite가 거리로 한다 — 이 컴포넌트는 입의 위치와 크기를 들고 있을 뿐이다
	MouthVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	MouthVolume->SetCanEverAffectNavigation(false);

}

void ASnake::BeginPlay()
{
	Super::BeginPlay();

	if (!SnakeData)
	{
		UE_LOG(LogTemp, Error, TEXT("Snake %s has no SnakeData."), *GetName());
		SetActorTickEnabled(false);
		return;
	}

	// 모드가 하나라도 빠지면 추첨에서 뽑히고도 설정을 못 찾는다. 여기서 한 번만 검증한다
	for (uint8 Value = 0; Value <= static_cast<uint8>(ESnakeMode::Lurk); ++Value)
	{
		const ESnakeMode Mode = static_cast<ESnakeMode>(Value);
		const bool bDefined = SnakeData->Modes.ContainsByPredicate(
			[Mode](const FSnakeModeSettings& Settings) { return Settings.Mode == Mode; });

		if (!bDefined)
		{
			UE_LOG(LogTemp, Error, TEXT("Snake %s: SnakeData is missing mode %d."), *GetName(), Value);
			SetActorTickEnabled(false);
			return;
		}
	}

	// 목표점 사이의 현이 TargetReachRadius보다 짧으면 뱀이 제자리에서 돈다
	const float CoilChord = 2.0f * SnakeData->CoilMinRadius
		* FMath::Sin(FMath::DegreesToRadians(SnakeData->OrbitStepDeg * 0.5f));
	if (CoilChord <= SnakeData->TargetReachRadius)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Snake %s: CoilMinRadius %.0f is too small for OrbitStepDeg %.0f — chord %.0f <= TargetReachRadius %.0f. The snake will spin in place at the center."),
			*GetName(), SnakeData->CoilMinRadius, SnakeData->OrbitStepDeg, CoilChord, SnakeData->TargetReachRadius);
	}

	HeadMesh->SetRelativeScale3D(FVector(SnakeData->HeadMeshScale));

	// 매치 시작 연출: 땅속에 묻힌 채로 시작해 StartGraceDuration 뒤에 솟아오른다.
	// 자취 프리필과 머리 높이가 모두 이 값을 보므로 여기서 먼저 정한다
	SubmergeAlpha = 1.0f;
	bWantSubmerged = true;

	// 레벨에 놓인 위치에서 반경과 방위를 읽어 아레나 위로 다시 앉힌다
	const FVector SpawnLocation = GetActorLocation();
	OrbitRadius = FMath::Clamp(SpawnLocation.Size2D(), SnakeData->MinOrbitRadius, SnakeData->MaxOrbitRadius);
	OrbitDirection = FMath::RandBool() ? 1.0f : -1.0f;

	const float SpawnAngleDeg = FMath::RadiansToDegrees(FMath::Atan2(SpawnLocation.Y, SpawnLocation.X));
	FVector HeadLocation = PointOnArena(SpawnAngleDeg, OrbitRadius);
	HeadLocation.Z = SurfaceZ(HeadLocation);
	SetActorLocation(HeadLocation);

	// 순회 접선 방향을 보게 둔다. 안 그러면 첫 목표점을 향해 크게 휘돌며 시작한다
	SetActorRotation(FRotator(0.0f, SpawnAngleDeg + 90.0f * OrbitDirection, 0.0f));

	// 흙 둔덕이 높이를 물어볼 지형. 하나만 있으면 된다
	for (TActorIterator<ALandscapeProxy> It(GetWorld()); It; ++It)
	{
		Landscape = *It;
		break;
	}

	if (!Landscape.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Snake %s: no landscape found. Mounds will follow props."), *GetName());
	}

	BuildSegmentLayout();

	// 머리 판정은 데이터가 준 값 그대로다. 스켈레탈 메시의 바운드는 PhysicsAsset과
	// Bounds Extension이 섞여 있어 보이는 크기와 다르므로 거기서 뽑지 않는다
	MouthVolume->SetSphereRadius(SnakeData->MouthRadius);
	MouthVolume->SetRelativeLocation(FVector(SnakeData->MouthForwardOffset, 0.0f, 0.0f));

	for (int32 Index = 0; Index < SnakeData->SegmentCount; ++Index)
	{
		SegmentMeshes.Add(CreateSegmentMesh(SnakeData->SegmentMesh, SnakeData->SegmentMaterial));
		MoundMeshes.Add(CreateSegmentMesh(SnakeData->MoundMesh, SnakeData->MoundMaterial));
	}

	// 발사체·트레이스는 몸통을 통과한다 — 몸통이 탄을 먹으면 슈팅이 뱀 뒤에서 무력해진다.
	// 막는 대상은 반죽뿐이고, 그건 PushOutTrappedCharacters가 이 데이터로 한다
	SegmentBodies.SetNum(SnakeData->SegmentCount);

	PrefillTrail();
	BeginMode(ESnakeMode::Orbit);
	UpdateSegments();
}

void ASnake::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 종료 프리즈 — 결과 화면 뒤에서 뱀이 계속 기어다니지 않게 한다
	if (IsMatchFinished())
	{
		return;
	}

	MatchElapsed += DeltaSeconds;

	if (TelegraphRemaining > 0.0f)
	{
		// 예고 동안은 모드 시간이 흐르지 않는다. 목표점은 이미 정해져 있고 머리가 그쪽을 향한다
		TelegraphRemaining -= DeltaSeconds;
		if (TelegraphRemaining <= 0.0f)
		{
			BP_OnTelegraphFinished();
		}
	}
	else
	{
		ModeElapsed += DeltaSeconds;

		if (IsModeFinished())
		{
			BeginMode(PickNextMode());
		}
		else if (FVector::Dist2D(GetActorLocation(), TargetPoint) <= SnakeData->TargetReachRadius)
		{
			UpdateTarget();
		}
	}

	const bool bTelegraphing = TelegraphRemaining > 0.0f;
	const float Speed = bTelegraphing
		? SnakeData->BaseSpeed * SnakeData->TelegraphSpeedMultiplier
		: CurrentSpeed;
	const float TurnRateDeg = bTelegraphing ? SnakeData->TelegraphTurnRateDeg : SnakeData->MaxTurnRateDeg;

	UpdateSubmergeState(DeltaSeconds, bTelegraphing);
	MoveHead(DeltaSeconds, Speed, TurnRateDeg);
	UpdateSubmergeTrail(DeltaSeconds);
	PushTrailSample();
	UpdateSegments();
	UpdateBite(DeltaSeconds);
	PushOutTrappedCharacters();

	if (bDrawDebug)
	{
		UWorld* World = GetWorld();
		const FColor Color = bTelegraphing ? FColor::Red : FColor::Green;
		DrawDebugSphere(World, TargetPoint, 150.0f, 12, Color, false, -1.0f, 0, 4.0f);
		DrawDebugLine(World, GetActorLocation(), TargetPoint, Color, false, -1.0f, 0, 2.0f);

		for (int32 Index = 4; Index < Trail.Num(); Index += 4)
		{
			DrawDebugLine(World, Trail[Index - 4].Location, Trail[Index].Location, FColor::Orange, false, -1.0f, 0, 1.0f);
		}
	}
}

// ---------------------------------------------------------------------------
// 모드
// ---------------------------------------------------------------------------

void ASnake::BeginMode(ESnakeMode NewMode)
{
	const FSnakeModeSettings& Settings = FindModeSettings(NewMode);

	CurrentMode = NewMode;
	ModeElapsed = 0.0f;
	ModeDuration = FMath::RandRange(Settings.MinDuration, Settings.MaxDuration);
	CurrentSpeed = SnakeData->BaseSpeed * Settings.SpeedMultiplier;

	if (NewMode == ESnakeMode::Orbit || NewMode == ESnakeMode::Coil)
	{
		// 직전 모드가 끝난 자리의 반경을 그대로 물려받는다 (사양 3.2 "현재 반경 유지")
		const float Lowest = NewMode == ESnakeMode::Coil ? SnakeData->CoilMinRadius : SnakeData->MinOrbitRadius;
		OrbitRadius = FMath::Clamp(GetActorLocation().Size2D(), Lowest, SnakeData->MaxOrbitRadius);

		// 대개 직전 방향을 유지한다. 매번 새로 뽑으면 절반은 반대로 나와 제자리 U턴을 하고,
		// 그때 머리가 자기 몸통을 관통한다 (U턴 폭 = 선회 반경 x 2)
		if (FMath::FRand() < SnakeData->OrbitReverseChance)
		{
			OrbitDirection = -OrbitDirection;
		}
	}

	if (NewMode == ESnakeMode::Coil)
	{
		CoilTurnedDeg = 0.0f;
		CoilInward = 1.0f;
	}

	// 잠수 주기는 모드마다 새로 센다. 모드 경계에서 구멍이 몰리지 않게 한다
	SubmergeRemaining = 0.0f;
	SubmergeTimer = FMath::RandRange(SnakeData->SubmergeIntervalMin, SnakeData->SubmergeIntervalMax);

	UpdateTarget();

	// 목표를 정한 다음에 예고한다 — 예고에서 보여준 방향이 실제로 갈 방향이어야 한다
	TelegraphRemaining = Settings.TelegraphDuration;
	if (TelegraphRemaining > 0.0f)
	{
		BP_OnTelegraphStarted(NewMode, TargetPoint);
	}

	BP_OnModeChanged(NewMode);
}

ESnakeMode ASnake::PickNextMode() const
{
	// 규칙 2 — 횡단·잠복 뒤에는 반드시 순회 (사양 3.3). 숨 고르기가 없으면
	// 도망자가 쉴 틈 없이 사냥당한다
	if (CurrentMode == ESnakeMode::Cross || CurrentMode == ESnakeMode::Lurk)
	{
		return ESnakeMode::Orbit;
	}

	// 삼킨 것이 있으면 잠복을 뽑지 않는다. 잠복은 전 구간이 잠수라
	// 혹이 땅속으로 들어가고 뱉을 자리가 지형 아래가 된다 (사양 5.4 ③-b)
	const bool bSkipLurk = IsCarryingSwallowed();
	auto IsCandidate = [this, bSkipLurk](ESnakeMode Mode)
	{
		return Mode != CurrentMode && !(bSkipLurk && Mode == ESnakeMode::Lurk);
	};

	// 규칙 1 — 같은 모드 연속 금지
	float TotalWeight = 0.0f;
	for (const FSnakeModeSettings& Settings : SnakeData->Modes)
	{
		if (IsCandidate(Settings.Mode))
		{
			TotalWeight += Settings.SelectionWeight;
		}
	}

	float Roll = FMath::FRandRange(0.0f, TotalWeight);
	for (const FSnakeModeSettings& Settings : SnakeData->Modes)
	{
		if (!IsCandidate(Settings.Mode))
		{
			continue;
		}

		Roll -= Settings.SelectionWeight;
		if (Roll <= 0.0f)
		{
			return Settings.Mode;
		}
	}

	// 가중치가 전부 0이면 추첨이 성립하지 않는다. 연속 금지만 지킨다
	return CurrentMode == ESnakeMode::Orbit ? ESnakeMode::Cross : ESnakeMode::Orbit;
}

const FSnakeModeSettings& ASnake::FindModeSettings(ESnakeMode Mode) const
{
	const FSnakeModeSettings* Found = SnakeData->Modes.FindByPredicate(
		[Mode](const FSnakeModeSettings& Settings) { return Settings.Mode == Mode; });

	// BeginPlay에서 네 모드의 존재를 검증했다
	check(Found);
	return *Found;
}

bool ASnake::IsModeFinished() const
{
	switch (CurrentMode)
	{
	case ESnakeMode::Cross:
		return FVector::Dist2D(GetActorLocation(), TargetPoint) <= SnakeData->TargetReachRadius;

	case ESnakeMode::Coil:
		return CoilTurnedDeg >= SnakeData->CoilRevolutions * 360.0f;

	case ESnakeMode::Lurk:
	{
		// 최소 잠복 시간을 못 채우면 아직 안 끝났다. 겨냥한 반죽이 코앞에 있거나
		// 아예 없을 때 한 프레임 만에 솟아오르는 것을 막는다 —
		// 예고로 "땅에 들어간다"를 1.5초 보여주고 즉시 튀어나오면 예고가 거짓말이 된다
		if (ModeElapsed < FindModeSettings(ESnakeMode::Lurk).MinDuration)
		{
			return false;
		}

		// 겨냥한 반죽에 닿거나, 시간이 다 되면 그 자리에서 솟는다
		return FVector::Dist2D(GetActorLocation(), TargetPoint) <= SnakeData->LurkTargetReachRadius
			|| ModeElapsed >= ModeDuration;
	}

	default:
		return ModeElapsed >= ModeDuration;
	}
}

void ASnake::UpdateTarget()
{
	const float AngleDeg = CurrentAngleDeg();

	switch (CurrentMode)
	{
	case ESnakeMode::Cross:
	{
		const float SpreadDeg = FMath::FRandRange(-SnakeData->CrossAngleSpreadDeg, SnakeData->CrossAngleSpreadDeg);
		// 목표 반경이 MaxOrbitRadius를 넘으면 머리가 클램프에 걸려 영원히 닿지 못한다
		const float Radius = FMath::Min(
			FMath::FRandRange(SnakeData->CrossRadiusMin, SnakeData->CrossRadiusMax),
			SnakeData->MaxOrbitRadius);

		TargetPoint = PointOnArena(AngleDeg + 180.0f + SpreadDeg, Radius);
		break;
	}

	case ESnakeMode::Coil:
	{
		// 한 바퀴에 CoilStepIn만큼 줄어들도록 스텝 각도에 비례해 깎는다
		OrbitRadius -= SnakeData->CoilStepIn * (SnakeData->OrbitStepDeg / 360.0f) * CoilInward;
		if (CoilInward > 0.0f && OrbitRadius <= SnakeData->CoilMinRadius)
		{
			// 다 감겼으면 같은 폭으로 풀면서 나온다
			OrbitRadius = SnakeData->CoilMinRadius;
			CoilInward = -1.0f;
		}
		OrbitRadius = FMath::Clamp(OrbitRadius, SnakeData->CoilMinRadius, SnakeData->MaxOrbitRadius);

		CoilTurnedDeg += SnakeData->OrbitStepDeg;
		TargetPoint = PointOnArena(AngleDeg + SnakeData->OrbitStepDeg * OrbitDirection, OrbitRadius);
		break;
	}

	case ESnakeMode::Lurk:
		TargetPoint = FindNearestPlayerLocation();
		break;

	default:
		TargetPoint = PointOnArena(AngleDeg + SnakeData->OrbitStepDeg * OrbitDirection, OrbitRadius);
		break;
	}

	// 목표는 평면 위의 점이다. 높이는 지형이 정한다
	TargetPoint.Z = GetActorLocation().Z;
}

FVector ASnake::FindNearestPlayerLocation() const
{
	const FVector HeadLocation = GetActorLocation();

	const ACSKPlayerCharacter* Nearest = nullptr;
	float NearestDistanceSq = TNumericLimits<float>::Max();

	for (TActorIterator<ACSKPlayerCharacter> It(GetWorld()); It; ++It)
	{
		const float DistanceSq = FVector::DistSquared2D(It->GetActorLocation(), HeadLocation);
		if (DistanceSq < NearestDistanceSq)
		{
			NearestDistanceSq = DistanceSq;
			Nearest = *It;
		}
	}

	if (!Nearest)
	{
		// 반죽이 하나도 없다. 잠복할 이유가 없으므로 제자리를 목표로 두고 시간으로 끝낸다
		return HeadLocation;
	}

	FVector Target = Nearest->GetActorLocation();
	const float Radius = Target.Size2D();
	if (Radius > SnakeData->MaxOrbitRadius)
	{
		const float Scale = SnakeData->MaxOrbitRadius / Radius;
		Target.X *= Scale;
		Target.Y *= Scale;
	}

	return Target;
}

void ASnake::UpdateSubmergeState(float DeltaSeconds, bool bTelegraphing)
{
	bool bWant = false;

	// 매치 시작 유예: 아직 땅속에 있다. 이 시간이 끝나면 저절로 솟아오르고
	// BP_OnSurfaced가 흙 터지는 연출을 받는다
	if (MatchElapsed < SnakeData->StartGraceDuration)
	{
		bWant = true;
	}
	// 잠복의 예고는 "머리가 땅에 박히는 게 보이는" 동작이다. 예고 동안은 아직 지상에 있다
	else if (bTelegraphing)
	{
		bWant = false;
	}
	else if (CurrentMode == ESnakeMode::Lurk)
	{
		bWant = true;
	}
	else if (IsCarryingSwallowed())
	{
		// 삼킨 것을 넘길 때까지 땅에 들어가지 않는다 (사양 5.4 ③-b).
		// 타이머를 멈춰두므로 다 뱉으면 그 자리에서 이어진다
		bWant = false;
	}
	else if (CurrentMode == ESnakeMode::Orbit || CurrentMode == ESnakeMode::Coil)
	{
		// 순회·똬리 중에만 주기적으로 들락거린다. 횡단은 자른 길이 유지돼야 의미가 있다
		if (SubmergeRemaining > 0.0f)
		{
			SubmergeRemaining -= DeltaSeconds;
			bWant = SubmergeRemaining > 0.0f;
			if (!bWant)
			{
				SubmergeTimer = FMath::RandRange(SnakeData->SubmergeIntervalMin, SnakeData->SubmergeIntervalMax);
			}
		}
		else
		{
			SubmergeTimer -= DeltaSeconds;
			if (SubmergeTimer <= 0.0f)
			{
				SubmergeRemaining = FMath::RandRange(SnakeData->SubmergeDurationMin, SnakeData->SubmergeDurationMax);
				bWant = true;
			}
		}
	}

	if (bWant != bWantSubmerged)
	{
		bWantSubmerged = bWant;

		// 전환이 끝난 뒤가 아니라 시작할 때 알린다. 흙은 파기 시작할 때 터져야 한다
		const FVector Location = GetActorLocation();
		const FVector GroundLocation(Location.X, Location.Y, Location.Z - CurrentSurfaceOffset());
		if (bWantSubmerged)
		{
			BP_OnSubmerged(GroundLocation);
		}
		else
		{
			BP_OnSurfaced(GroundLocation);
		}
	}

	// 솟기 직전에 나올 자리를 찍는다. 잠수는 몸이 안 보이므로 이 예고가 없으면
	// 머리 위치를 알 수 없고, 그러면 피할 수단이 아예 없다 (사양 1.2)
	if (!bWantSubmerged)
	{
		bSurfaceTelegraphed = false;
	}
	else if (!bSurfaceTelegraphed && IsSurfacingSoon())
	{
		bSurfaceTelegraphed = true;

		const FVector HeadLocation = GetActorLocation();
		BP_OnSurfaceTelegraph(
			FVector(HeadLocation.X, HeadLocation.Y, HeadLocation.Z - CurrentSurfaceOffset()));
	}

	// 목표를 향해 진행도를 움직인다. 이 값이 시간을 갖기 때문에 파고드는 동작이 보인다
	const float Step = DeltaSeconds / FMath::Max(SnakeData->SubmergeTransitionDuration, UE_SMALL_NUMBER);
	SubmergeAlpha = FMath::Clamp(SubmergeAlpha + (bWantSubmerged ? Step : -Step), 0.0f, 1.0f);
}

bool ASnake::IsSurfacingSoon() const
{
	const float Lead = SnakeData->SurfaceTelegraphDuration;
	if (Lead <= 0.0f)
	{
		return false;
	}

	// 매치 시작 유예는 시간만 보면 된다. 끝나는 순간 그 자리에서 솟는다
	if (MatchElapsed < SnakeData->StartGraceDuration)
	{
		return SnakeData->StartGraceDuration - MatchElapsed <= Lead;
	}

	// 잠복은 겨냥한 반죽에 닿거나 시간이 다 되면 솟는다. 둘 중 먼저 오는 쪽으로 잰다
	if (CurrentMode == ESnakeMode::Lurk)
	{
		if (ModeElapsed < FindModeSettings(ESnakeMode::Lurk).MinDuration)
		{
			return false;
		}

		const float DistanceToSurface =
			FVector::Dist2D(GetActorLocation(), TargetPoint) - SnakeData->LurkTargetReachRadius;
		return DistanceToSurface <= CurrentSpeed * Lead || ModeDuration - ModeElapsed <= Lead;
	}

	// 순회·똬리의 주기 잠수는 남은 시간이 전부다
	return SubmergeRemaining > 0.0f && SubmergeRemaining <= Lead;
}

// ③ 땅속을 달리는 자국. 이게 없으면 잠수한 뱀이 "사라진 것"으로 보인다
void ASnake::UpdateSubmergeTrail(float DeltaSeconds)
{
	if (CurrentSurfaceOffset() >= 0.0f)
	{
		SubmergeTrailTimer = 0.0f;
		return;
	}

	SubmergeTrailTimer -= DeltaSeconds;
	if (SubmergeTrailTimer > 0.0f)
	{
		return;
	}

	SubmergeTrailTimer = SnakeData->SubmergeTrailInterval;

	const FVector Location = GetActorLocation();
	BP_OnSubmergeTrail(FVector(Location.X, Location.Y, Location.Z - CurrentSurfaceOffset()));
}

// ---------------------------------------------------------------------------
// 머리
// ---------------------------------------------------------------------------

void ASnake::MoveHead(float DeltaSeconds, float Speed, float TurnRateDeg)
{
	const FVector OldLocation = GetActorLocation();

	float NewYaw = GetActorRotation().Yaw;

	const FVector ToTarget = (TargetPoint - OldLocation).GetSafeNormal2D();
	if (!ToTarget.IsNearlyZero())
	{
		NewYaw = FMath::FixedTurn(NewYaw, ToTarget.Rotation().Yaw, TurnRateDeg * DeltaSeconds);
	}

	// 전진은 수평 방향으로만 한다. 기울어진 forward로 밀면 파고드는 동안 수평 속도가
	// cos(기울기)만큼 줄어 뱀이 제자리에서 가라앉는다
	const FVector Forward = FRotator(0.0f, NewYaw, 0.0f).Vector();
	FVector NewLocation = OldLocation + Forward * Speed * DeltaSeconds;

	// 오목 벽(밴드 D)을 타고 오르지 않게 한다 (사양 3.2)
	const float Radius = NewLocation.Size2D();
	if (Radius > SnakeData->MaxOrbitRadius)
	{
		const float Scale = SnakeData->MaxOrbitRadius / Radius;
		NewLocation.X *= Scale;
		NewLocation.Y *= Scale;
	}

	NewLocation.Z = SurfaceZ(NewLocation);
	SetActorLocation(NewLocation);

	// 실제로 움직인 방향으로 코를 든다/박는다. 잠수 전환의 Z 변화가 그대로 자세가 되므로
	// 파고드는 각도를 따로 계산하지 않는다 — 지형 경사에도 같은 규칙이 적용된다
	const FVector Delta = NewLocation - OldLocation;
	const float Horizontal = Delta.Size2D();
	const float DesiredPitch = FMath::Clamp(
		FMath::RadiansToDegrees(FMath::Atan2(Delta.Z, Horizontal)),
		-SnakeData->MaxPitchDeg,
		SnakeData->MaxPitchDeg);

	CurrentPitch = FMath::FInterpTo(CurrentPitch, DesiredPitch, DeltaSeconds, SnakeData->PitchInterpSpeed);
	SetActorRotation(FRotator(CurrentPitch, NewYaw, 0.0f));

	// 클램프된 뒤의 실제 이동량으로 누적한다. 안 그러면 자취의 거리축이 실제 곡선과 어긋난다
	HeadDistance += FVector::Dist(OldLocation, NewLocation);
}

// ---------------------------------------------------------------------------
// 자취와 몸통
// ---------------------------------------------------------------------------

void ASnake::PushTrailSample()
{
	if (HeadDistance - Trail.Last().DistanceAlong < SnakeData->TrailSampleDistance)
	{
		return;
	}

	const FVector HeadLocation = GetActorLocation();
	const float Offset = CurrentSurfaceOffset();
	Trail.Add({ HeadLocation, HeadDistance, Offset, TerrainZ(HeadLocation) });

	// 꼬리 뒤로 넘어간 샘플은 버린다
	const float OldestNeeded = HeadDistance - RequiredTrailLength();
	int32 DropCount = 0;
	while (DropCount + 1 < Trail.Num() && Trail[DropCount + 1].DistanceAlong <= OldestNeeded)
	{
		++DropCount;
	}

	if (DropCount > 0)
	{
		Trail.RemoveAt(0, DropCount, EAllowShrinking::No);
	}
}

void ASnake::UpdateSegments()
{
	const int32 SegmentCount = SnakeData->SegmentCount;

	int32 SearchIndex = Trail.Num() - 1;
	FVector AheadLocation = GetActorLocation();

	for (int32 Index = 0; Index < SegmentCount; ++Index)
	{
		const float Distance = HeadDistance - SegmentDistances[Index];
		const FSnakeTrailSample Sample = SampleTrail(Distance, SearchIndex);
		const FVector Location = Sample.Location;

		// 마디는 자기 앞 마디를 바라본다. 구가 아닌 메시를 꽂아도 방향이 맞는다
		const FVector Forward = AheadLocation - Location;
		const FRotator Rotation = Forward.IsNearlyZero() ? GetActorRotation() : Forward.Rotation();

		// 삼킨 것이 지나가는 마디를 부풀린다 — 밖에서 혹으로 보인다
		float Scale = SegmentScales[Index];
		for (const FSnakeSwallowed& Entry : Swallowed)
		{
			if (!Entry.bTraveling)
			{
				continue;
			}

			const float BulgeDistance = Entry.TravelStartDistance - (HeadDistance - Entry.TravelStartDistance);
			const float Falloff = 1.0f - FMath::Abs(Distance - BulgeDistance) / SnakeData->SegmentSpacing;
			if (Falloff > 0.0f)
			{
				Scale = FMath::Max(Scale,
					SegmentScales[Index] * FMath::Lerp(1.0f, SnakeData->SwallowBulgeScale, Falloff));
			}
		}

		// TeleportType::None 이어야 엔진이 모션 벡터를 만든다. 마디는 연속으로 움직인다
		SegmentMeshes[Index]->SetWorldTransform(
			FTransform(Rotation, Location, FVector(Scale)), /*bSweep*/ false, nullptr, ETeleportType::None);

		// 잠긴 마디는 막지 않는다. 이 구멍이 몸을 따라 뒤로 흘러간다 (사양 3.5).
		// 기준이 마디마다 다르다 — 가는 꼬리는 얕게만 내려가도 지면 아래로 들어간다
		const bool bSegmentHidden = Sample.SurfaceOffset <= -SegmentRadii[Index];

		FSnakeSegmentBody& Body = SegmentBodies[Index];
		Body.Location = Location;
		// 보이는 크기와 막히는 크기는 혹이 지나갈 때도 같아야 한다
		Body.Radius = SegmentRadii[Index] * Scale / SegmentScales[Index];
		Body.bBlocking = !bSegmentHidden;

		// 잠긴 마디 위 지면에 둔덕을 올린다. 이랑이 곧 "땅속에 뭔가 지나간다"는 표시이고,
		// 이랑의 양 끝이 구멍의 입구·출구다
		UStaticMeshComponent* Mound = MoundMeshes[Index];
		Mound->SetVisibility(bSegmentHidden);
		if (bSegmentHidden)
		{
			const float Width = SegmentScales[Index] * SnakeData->MoundWidthScale;
			const float Height = SegmentScales[Index] * SnakeData->MoundHeightScale;

			// 자기 높이만큼 지면 아래로 묻는다. 안 묻으면 반구가 드러나 과자처럼 보이고,
			// 묻어야 이웃과 겹쳐 하나의 이랑으로 읽힌다
			const float MeshRadius = SegmentRadii[Index] / FMath::Max(SegmentScales[Index], UE_SMALL_NUMBER);
			const float Sink = MeshRadius * Height * SnakeData->MoundSinkRatio;
			const FVector Ground(Location.X, Location.Y, Sample.GroundZ - Sink);

			// 지면 이랑은 눕혀야 한다. 마디 회전을 그대로 쓰면 파고드는 각도로 기울어진다
			Mound->SetWorldTransform(
				FTransform(FRotator(0.0f, Rotation.Yaw, 0.0f), Ground, FVector(Width, Width, Height)),
				/*bSweep*/ false, nullptr, ETeleportType::None);
		}

		AheadLocation = Location;
	}
}

UStaticMeshComponent* ASnake::CreateSegmentMesh(UStaticMesh* Mesh, UMaterialInterface* Material)
{
	UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(this);
	Component->SetupAttachment(SceneRoot);
	Component->SetStaticMesh(Mesh);
	if (Material)
	{
		Component->SetMaterial(0, Material);
	}
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component->SetCanEverAffectNavigation(false);
	// 마디 위치는 자취가 정한다. 루트(머리)를 따라다니면 안 된다
	Component->SetUsingAbsoluteLocation(true);
	Component->SetUsingAbsoluteRotation(true);
	Component->SetUsingAbsoluteScale(true);
	Component->RegisterComponent();

	return Component;
}

void ASnake::BuildSegmentLayout()
{
	const int32 Count = SnakeData->SegmentCount;
	const float HeadScale = SnakeData->SegmentMeshScale;
	const float TailScale = HeadScale * SnakeData->TailScaleRatio;

	// 메시의 실제 반경. 앞을 보도록 회전시키므로 X가 길이, Y/Z가 굵기다
	const UStaticMesh* Mesh = SnakeData->SegmentMesh;
	const FVector Extent = Mesh ? Mesh->GetBounds().BoxExtent : FVector::ZeroVector;
	const float MeshRadius = FMath::Max(Extent.Y, Extent.Z);

	SegmentScales.SetNum(Count);
	SegmentRadii.SetNum(Count);
	SegmentDistances.SetNum(Count);

	float Distance = SnakeData->HeadSegmentGap;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const float Alpha = Count > 1 ? static_cast<float>(Index) / (Count - 1) : 0.0f;
		const float Scale = FMath::Lerp(HeadScale, TailScale, Alpha);

		if (Index > 0)
		{
			// 간격을 굵기에 비례시킨다. 고정 간격이면 가늘어진 꼬리에서 메시가 끊긴다.
			// 앞뒤 두 마디의 평균 굵기를 쓰므로 경계에서 튀지 않는다
			const float Ratio = (Scale + SegmentScales[Index - 1]) * 0.5f / FMath::Max(HeadScale, UE_SMALL_NUMBER);
			Distance += SnakeData->SegmentSpacing * Ratio;
		}

		SegmentScales[Index] = Scale;
		SegmentRadii[Index] = MeshRadius * Scale;
		SegmentDistances[Index] = Distance;
	}
}

// ---------------------------------------------------------------------------
// 포식. 사양 5장
// ---------------------------------------------------------------------------

void ASnake::UpdateBite(float DeltaSeconds)
{
	for (int32 Index = BiteImmunities.Num() - 1; Index >= 0; --Index)
	{
		BiteImmunities[Index].Remaining -= DeltaSeconds;
		if (BiteImmunities[Index].Remaining <= 0.0f || !BiteImmunities[Index].Character.IsValid())
		{
			BiteImmunities.RemoveAtSwap(Index);
		}
	}

	for (int32 Index = Swallowed.Num() - 1; Index >= 0; --Index)
	{
		FSnakeSwallowed& Entry = Swallowed[Index];
		ACSKPlayerCharacter* Character = Entry.Character.Get();
		if (!Character)
		{
			Swallowed.RemoveAt(Index);
			continue;
		}

		if (!Entry.bTraveling)
		{
			// 입에 물려 있는 동안
			Entry.SwallowElapsed += DeltaSeconds;
			Character->SetActorLocation(MouthVolume->GetComponentLocation());

			if (Entry.SwallowElapsed >= SnakeData->SwallowDuration)
			{
				Entry.bTraveling = true;
				Entry.TravelStartDistance = HeadDistance;
			}
			continue;
		}

		const float Progress = HeadDistance - Entry.TravelStartDistance;
		if (Progress >= SnakeData->SwallowTravelDistance)
		{
			SpitOut(Entry);
			Swallowed.RemoveAt(Index);
			continue;
		}

		// 자취를 거슬러 꼬리 쪽으로 밀려간다
		int32 SearchIndex = Trail.Num() - 1;
		Character->SetActorLocation(SampleTrail(Entry.TravelStartDistance - Progress, SearchIndex).Location);
	}

	const FVector MouthLocation = MouthVolume->GetComponentLocation();
	for (TActorIterator<ACSKPlayerCharacter> It(GetWorld()); It; ++It)
	{
		ACSKPlayerCharacter* Character = *It;
		if (!CanBite(Character))
		{
			continue;
		}

		const float Reach = SnakeData->MouthRadius + Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
		if (FVector::Dist(MouthLocation, Character->GetActorLocation()) <= Reach)
		{
			BeginSwallow(Character);
		}
	}
}

bool ASnake::CanBite(const ACSKPlayerCharacter* Character) const
{
	if (!Character)
	{
		return false;
	}

	// 땅속에 있는 머리는 삼키지 않는다. 볼 수 없는 공격으로 몸집을 뺏으면 100% 운이 된다 (사양 5.3).
	// 기준은 진행도가 아니라 **머리가 실제로 지면 위에 있는지**다 —
	// 알파로 보면 0.1만 잠겨도(= 눈에는 멀쩡히 지상) 못 무는 시간이 길어진다
	if (CurrentSurfaceOffset() <= 0.0f)
	{
		return false;
	}

	// 킹은 예외가 아니다 — 킹도 삼켜진다
	if (Character->IsSequenceLocked() || Character->IsBeingEaten())
	{
		return false;
	}

	for (const FSnakeBiteImmunity& Immunity : BiteImmunities)
	{
		if (Immunity.Character.Get() == Character)
		{
			return false;
		}
	}

	return !IsSwallowed(Character);
}

void ASnake::BeginSwallow(ACSKPlayerCharacter* Character)
{
	const FVector BiteLocation = Character->GetActorLocation();

	// 삼켜지는 쪽은 화면이 뱀으로 넘어가서 자기 일인 줄 알지만, 남들은 한 명이 사라진 것만 본다.
	// 누가 물렸는지가 전황의 절반이다
	if (ACSKGameState* GameState = GetWorld()->GetGameState<ACSKGameState>())
	{
		GameState->BroadcastMatchEvent(
			FCSKMatchEventInfo(ECSKMatchEventType::SwallowedBySnake, Character->GetPlayerState()));
	}

	// 절구공이는 삼키는 순간 그 자리에 떨어진다 — 뱀 반대편에 남는다
	if (APestleItem* Pestle = Character->GetCarriedPestleItem())
	{
		Pestle->DropFromCarrier((BiteLocation - GetActorLocation()).GetSafeNormal2D());
	}

	ScatterMass(Character, BiteLocation);

	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();

	FSnakeSwallowed Entry;
	Entry.Character = Character;
	Entry.SavedCollision = Capsule->GetCollisionEnabled();
	Swallowed.Add(Entry);

	// 몸통 구체는 WorldDynamic이다. Pawn 응답만 꺼두면 삼켜진 반죽이 몸통 안에서
	// 계속 침투 판정을 일으키고, CharacterMovement가 그걸 해소하려다 지형을 뚫고 추락한다
	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 킹에게 먹힐 때와 같은 처리 (UCSKEatableComponent::TryStartEatReaction).
	// 도약 취소, 조준 해제, 소프트바디 리셋, 애님 물리 리셋을 한 번에 한다
	Character->NotifyBeingEatenStarted();

	if (AController* Controller = Character->GetController())
	{
		// 시점 입력까지 막는다. SetSequenceLocked는 이동 입력만 막는다
		Controller->SetIgnoreLookInput(true);

		// 시점을 뱀에게 넘긴다. 안 넘기면 카메라가 반죽을 따라 몸통 속·지형 아래로 들어간다
		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			PlayerController->SetViewTargetWithBlend(this, 0.2f);
		}
	}

	// 이동 정지·도약 취소·봇 로직 정지는 SetSequenceLocked가 처리한다
	Character->SetActorHiddenInGame(true);
	Character->SetSequenceLocked(true);

	BP_OnDevoured(Character, BiteLocation);
}

void ASnake::ScatterMass(ACSKPlayerCharacter* Character, const FVector& BiteLocation)
{
	UCSKPlayerScaleComponent* Scale = Character->GetScaleComponent();
	if (!Scale)
	{
		return;
	}

	// 현재 몸집이 아니라 최저치 위의 여유분에서 뜯는다 (킹과 같은 규칙).
	// 몸집 비율이면 서너 번 만에 바닥이고, 바닥에 닿은 반죽은 뱀을 무시하게 된다
	const float Removed = Scale->RemoveMass(Scale->GetLossableMass() * SnakeData->BiteMassRatio);
	if (Removed <= 0.0f)
	{
		return;
	}

	const int32 Count = FMath::Max(SnakeData->BlobCount, 1);
	const float Each = Removed / Count;
	const FVector Right = GetActorRightVector().GetSafeNormal2D();

	for (int32 Index = 0; Index < Count; ++Index)
	{
		// 좌우로 반씩 나눠 뿌린다. 앞뒤로 뿌리면 뱀 몸통 아래 깔려서 줍지 못한다
		const float Side = Index < Count / 2 ? 1.0f : -1.0f;
		const float SpreadDeg = FMath::FRandRange(-SnakeData->BlobSideSpreadDeg, SnakeData->BlobSideSpreadDeg);
		const FVector Direction = (Right * Side).RotateAngleAxis(SpreadDeg, FVector::UpVector);

		ADoughBlob::SpawnBlob(this, BiteLocation, Character->DoughType, Each, Direction);
	}
}

bool ASnake::IsSwallowed(const ACSKPlayerCharacter* Character) const
{
	for (const FSnakeSwallowed& Entry : Swallowed)
	{
		if (Entry.Character.Get() == Character)
		{
			return true;
		}
	}

	return false;
}

void ASnake::SpitOut(FSnakeSwallowed& Entry)
{
	ACSKPlayerCharacter* Character = Entry.Character.Get();
	if (!Character)
	{
		return;
	}

	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	const float SpitDistance = Entry.TravelStartDistance - SnakeData->SwallowTravelDistance;

	int32 SearchIndex = Trail.Num() - 1;
	FVector SpitLocation = SampleTrail(SpitDistance, SearchIndex).Location;

	// 좌우 중 무작위
	const FVector Side = TrailRightAt(SpitDistance) * (FMath::RandBool() ? 1.0f : -1.0f);

	// **위치부터 몸통 밖으로 내보낸다.** 자취 위는 몸통 구체 안이고, 거기서 콜리전을 되살리면
	// 엔진이 침투를 해소하며 아래로 밀어 반죽이 지형을 뚫고 추락한다.
	// 사출 속도만 옆으로 줘서는 이미 박힌 자리를 빠져나오지 못한다
	const float Clearance =
		SegmentRadii[0] + Capsule->GetScaledCapsuleRadius() * 2.0f;
	SpitLocation += Side * Clearance;

	// 자취는 잠수 깊이를 품고 있다. 몸통 밖으로 나온 **뒤에** 그 자리의 지면에서 높이를 잡는다
	SpitLocation.Z = GroundZ(SpitLocation) + Capsule->GetScaledCapsuleHalfHeight();

	Character->SetActorHiddenInGame(false);
	Capsule->SetCollisionEnabled(Entry.SavedCollision);
	// 자리부터 잡고 잠금을 푼다 — 푸는 순간 이동이 살아나므로 옛 위치에서 한 프레임 낙하하지 않는다
	Character->SetActorLocation(SpitLocation, false, nullptr, ETeleportType::TeleportPhysics);
	Character->SetSequenceLocked(false);

	if (AController* Controller = Character->GetController())
	{
		Controller->SetIgnoreLookInput(false);

		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			PlayerController->SetViewTargetWithBlend(Character, 0.2f);
		}
	}

	// 몸통을 따라 먼 거리를 옮겨졌다. 리셋하지 않으면 젤리가 늘어난 채로 나온다
	if (UCSKPlayerSoftBodyComponent* SoftBody = Character->GetSoftBodyComponent())
	{
		SoftBody->ResetForCapture();
	}

	if (USkeletalMeshComponent* CharacterMesh = Character->GetMesh())
	{
		CharacterMesh->ResetAnimInstanceDynamics(ETeleportType::ResetPhysics);
	}

	Character->LaunchCharacter(
		Side * SnakeData->EjectSideStrength + FVector::UpVector * SnakeData->EjectUpStrength, true, true);

	FSnakeBiteImmunity Immunity;
	Immunity.Character = Character;
	Immunity.Remaining = SnakeData->BiteImmunityDuration;
	BiteImmunities.Add(Immunity);

	if (bDrawDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("Snake spit %s at Z=%.0f (ground %.0f)"),
			*Character->GetName(), SpitLocation.Z, SpitLocation.Z - Capsule->GetScaledCapsuleHalfHeight());
		DrawDebugSphere(GetWorld(), SpitLocation, 120.0f, 12, FColor::Cyan, false, 5.0f, 0, 4.0f);
	}

	BP_OnSpitOut(Character, SpitLocation);
}

FVector ASnake::TrailRightAt(float Distance) const
{
	int32 BackIndex = Trail.Num() - 1;
	const FVector Back = SampleTrail(Distance - 100.0f, BackIndex).Location;

	int32 FrontIndex = Trail.Num() - 1;
	const FVector Front = SampleTrail(Distance + 100.0f, FrontIndex).Location;

	const FVector Forward = (Front - Back).GetSafeNormal2D();
	if (Forward.IsNearlyZero())
	{
		return GetActorRightVector().GetSafeNormal2D();
	}

	return FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
}

void ASnake::PushOutTrappedCharacters()
{
	for (TActorIterator<ACSKPlayerCharacter> It(GetWorld()); It; ++It)
	{
		ACSKPlayerCharacter* Character = *It;
		const UCapsuleComponent* Body = Character->GetCapsuleComponent();

		// 삼켜진 반죽은 몸통 한가운데 있는 게 정상이다. 밀어내면 우리 텔레포트와 싸운다
		if (!Body || IsSwallowed(Character))
		{
			continue;
		}

		const FVector CharacterLocation = Character->GetActorLocation();
		const float CharacterRadius = Body->GetScaledCapsuleRadius();
		const float CharacterHalfHeight = Body->GetScaledCapsuleHalfHeight();

		// 몸통을 막는 일은 전부 여기서 한다. 몸통에는 엔진 콜리전이 없다 —
		// 권한이 하나여야 서로 밀어내며 떠는 일이 없다.
		// 가장 깊이 박힌 마디 하나만 처리한다. 겹친 마디를 전부 밀면
		// 방향이 서로 상쇄돼 오히려 제자리에 낀다
		int32 DeepestIndex = INDEX_NONE;
		float DeepestPenetration = 0.0f;

		for (int32 Index = 0; Index < SegmentBodies.Num(); ++Index)
		{
			const FSnakeSegmentBody& Segment = SegmentBodies[Index];
			if (!Segment.bBlocking)
			{
				continue;
			}

			// 몸통 위를 넘어가는 중이면 밀지 않는다. 도약으로 넘는 것은 막지 않는다
			const float VerticalReach = Segment.Radius + CharacterHalfHeight;
			if (FMath::Abs(CharacterLocation.Z - Segment.Location.Z) >= VerticalReach)
			{
				continue;
			}

			// 혹 때문에 커진 반지름을 그대로 쓴다
			const float Distance = FVector::Dist2D(Segment.Location, CharacterLocation);
			const float Penetration = (Segment.Radius + CharacterRadius) - Distance;
			if (Penetration > DeepestPenetration)
			{
				DeepestPenetration = Penetration;
				DeepestIndex = Index;
			}
		}

		if (DeepestIndex == INDEX_NONE)
		{
			continue;
		}

		FVector Outward = (CharacterLocation - SegmentBodies[DeepestIndex].Location).GetSafeNormal2D();
		if (Outward.IsNearlyZero())
		{
			// 마디 중심에 정확히 겹쳤다. 뱀의 진행 방향 옆으로 밀어낸다
			Outward = GetActorRightVector().GetSafeNormal2D();
		}

		// **정확히 표면까지만** 민다. 더 밀면 매 프레임 밖으로 튕겼다가 걸어 들어오기를
		// 반복하면서 떤다. 딱 맞추면 위치가 표면에 수렴해 멈춘 것처럼 보인다
		Character->AddActorWorldOffset(Outward * DeepestPenetration, /*bSweep*/ true);
	}
}

void ASnake::PrefillTrail()
{
	Trail.Reset();

	const float Length = RequiredTrailLength();
	const int32 SampleCount = FMath::CeilToInt(Length / SnakeData->TrailSampleDistance);

	// 스폰 반경의 원호를 따라 뒤로 되짚는다. 직선으로 채우면 몸통 끝이 아레나 밖으로 삐져나간다
	const float HeadAngleDeg = CurrentAngleDeg();
	const float StepDeg = FMath::RadiansToDegrees(SnakeData->TrailSampleDistance / OrbitRadius) * OrbitDirection;

	Trail.Reserve(SampleCount + 1);
	for (int32 Index = SampleCount; Index >= 1; --Index)
	{
		FVector Location = PointOnArena(HeadAngleDeg - StepDeg * Index, OrbitRadius);
		Location.Z = SurfaceZ(Location);

		Trail.Add({ Location, HeadDistance - SnakeData->TrailSampleDistance * Index,
			CurrentSurfaceOffset(), TerrainZ(Location) });
	}

	const FVector EndLocation = GetActorLocation();
	const float EndOffset = CurrentSurfaceOffset();
	Trail.Add({ EndLocation, HeadDistance, EndOffset, TerrainZ(EndLocation) });
}

float ASnake::RequiredTrailLength() const
{
	// 꼬리 마디가 보간할 구간이 남도록 한 칸 더 잡는다
	return SegmentDistances.Last() + SnakeData->SegmentSpacing;
}

ASnake::FSnakeTrailSample ASnake::SampleTrail(float Distance, int32& InOutIndex) const
{
	while (InOutIndex > 0 && Trail[InOutIndex].DistanceAlong > Distance)
	{
		--InOutIndex;
	}

	if (InOutIndex >= Trail.Num() - 1)
	{
		return Trail.Last();
	}

	const FSnakeTrailSample& Near = Trail[InOutIndex];
	const FSnakeTrailSample& Far = Trail[InOutIndex + 1];
	const float Span = Far.DistanceAlong - Near.DistanceAlong;
	const float Alpha = Span > UE_SMALL_NUMBER ? (Distance - Near.DistanceAlong) / Span : 0.0f;

	FSnakeTrailSample Result;
	Result.Location = FMath::Lerp(Near.Location, Far.Location, FMath::Clamp(Alpha, 0.0f, 1.0f));
	Result.DistanceAlong = Distance;
	// 두 샘플 중 더 깊은 쪽을 따른다 — 구멍 가장자리가 한 마디 넓어질 뿐,
	// 반대로 하면 구멍이 한 마디 좁아져 통과하려던 반죽이 벽에 부딪힌다
	Result.SurfaceOffset = FMath::Min(Near.SurfaceOffset, Far.SurfaceOffset);
	// 지면 높이는 그냥 보간한다. SurfaceOffset과 달리 구멍 판정과 무관하다
	Result.GroundZ = FMath::Lerp(Near.GroundZ, Far.GroundZ, FMath::Clamp(Alpha, 0.0f, 1.0f));
	return Result;
}

// ---------------------------------------------------------------------------
// 도우미
// ---------------------------------------------------------------------------

float ASnake::GroundZ(const FVector& Location) const
{
	const FVector Start = Location + FVector(0.0f, 0.0f, SnakeData->GroundTraceHeight);
	const FVector End = Location - FVector(0.0f, 0.0f, SnakeData->GroundTraceHeight);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SnakeGround), /*bTraceComplex*/ false, this);

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return static_cast<float>(Hit.ImpactPoint.Z);
	}

	return static_cast<float>(Location.Z);
}

float ASnake::TerrainZ(const FVector& Location) const
{
	// 트레이스로는 못 구한다 — LineTraceMulti도 첫 블로킹 히트(가마솥 뚜껑)에서 멈추기 때문에
	// 그 아래 흙까지 내려가지 않는다. 지형에 직접 묻는 것이 정석이다
	if (const ALandscapeProxy* Terrain = Landscape.Get())
	{
		const TOptional<float> Height = Terrain->GetHeightAtLocation(Location);
		if (Height.IsSet())
		{
			return Height.GetValue();
		}
	}

	// 지형 밖이다. 뱀이 딛는 높이로 대신한다
	return GroundZ(Location);
}

float ASnake::CurrentSurfaceOffset() const
{
	// 방향에 무관하게 대칭인 이징을 쓴다. 전환 도중에 목표가 뒤집혀도 값이 튀지 않는다
	const float Eased = FMath::SmoothStep(0.0f, 1.0f, SubmergeAlpha);
	return FMath::Lerp(SnakeData->HeightOffset, -SnakeData->SubmergeDepth, Eased);
}

float ASnake::SurfaceZ(const FVector& Location) const
{
	// 잠수는 메시를 숨기는 게 아니라 지형 아래로 내려가는 것이다. 지형이 뱀을 가린다
	return GroundZ(Location) + CurrentSurfaceOffset();
}

FVector ASnake::PointOnArena(float AngleDeg, float Radius) const
{
	// 아레나 중심 = 월드 원점 (Docs/ChapSalKing_LandscapeSpec.md 2장)
	const float AngleRad = FMath::DegreesToRadians(AngleDeg);
	return FVector(Radius * FMath::Cos(AngleRad), Radius * FMath::Sin(AngleRad), 0.0f);
}

float ASnake::CurrentAngleDeg() const
{
	const FVector Location = GetActorLocation();
	return FMath::RadiansToDegrees(FMath::Atan2(Location.Y, Location.X));
}

bool ASnake::IsMatchFinished() const
{
	const UWorld* World = GetWorld();
	const ACSKGameState* GameState = World ? World->GetGameState<ACSKGameState>() : nullptr;
	return GameState && GameState->IsMatchFinished();
}
