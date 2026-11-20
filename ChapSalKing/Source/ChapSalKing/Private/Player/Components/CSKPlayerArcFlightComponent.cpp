#include "Player/Components/CSKPlayerArcFlightComponent.h"

#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerMoveComponent.h"
#include "Player/Components/CSKPlayerSoftBodyComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

UCSKPlayerArcFlightComponent::UCSKPlayerArcFlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCSKPlayerArcFlightComponent::BeginArc(
	const FVector& Target,
	float Duration,
	float ArcHeight,
	bool bInEndInFalling,
	FCSKArcFlightFinished OnFinished)
{
	CacheOwnerCSKCharacter();

	if (bFlying)
	{
		Cancel();
	}

	ArcStart = OwnerCSKCharacter->GetActorLocation();
	ArcTarget = Target;
	Elapsed = 0.0f;
	TotalTime = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	ArcPeak = ArcHeight;
	bEndInFalling = bInEndInFalling;
	FinishedDelegate = MoveTemp(OnFinished);
	LastLocation = ArcStart;

	// 몸이 늘어나며 솟구치는 도약 반응 — 이동 물리를 끄기 전에 걸어야 한다
	if (UCSKPlayerSoftBodyComponent* SoftBody = OwnerCSKCharacter->GetSoftBodyComponent())
	{
		SoftBody->NotifyJumpTakeoff();
	}

	OwnerCSKCharacter->GetMoveComponent()->BeginLeapMovement();

	bFlying = true;
	SetComponentTickEnabled(true);
}

void UCSKPlayerArcFlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bFlying)
	{
		SetComponentTickEnabled(false);
		return;
	}

	Elapsed += DeltaTime;
	const float Alpha = FMath::Clamp(Elapsed / TotalTime, 0.0f, 1.0f);

	FVector Pos = FMath::Lerp(ArcStart, ArcTarget, Alpha);
	Pos.Z += ArcPeak * 4.0f * Alpha * (1.0f - Alpha);
	OwnerCSKCharacter->SetActorLocation(Pos, false);

	// 이동 모드가 MOVE_None이라 CMC는 위치를 건드리지 않는다.
	// Velocity는 순수히 "지금 얼마나 빠르게 어디로 가는가" 보고용 — 애니메이션·운반물 흔들림·이펙트가 이걸 읽는다
	if (UCharacterMovementComponent* Movement = OwnerCSKCharacter->GetCharacterMovement())
	{
		Movement->Velocity = (Pos - LastLocation) / FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);
	}
	LastLocation = Pos;

	if (Alpha >= 1.0f)
	{
		FinishArc();
	}
}

void UCSKPlayerArcFlightComponent::FinishArc()
{
	bFlying = false;
	SetComponentTickEnabled(false);

	// 도착 속도를 그대로 물려주면 착지가 뚝 끊기지 않고 이어진다
	if (bEndInFalling)
	{
		OwnerCSKCharacter->GetMoveComponent()->EndLeapMovement();
	}

	// 콜백 안에서 다음 BeginArc를 걸 수 있도록 델리게이트를 비운 뒤 실행
	FCSKArcFlightFinished Finished = MoveTemp(FinishedDelegate);
	FinishedDelegate.Unbind();
	Finished.ExecuteIfBound(ArcTarget);
}

void UCSKPlayerArcFlightComponent::Cancel()
{
	if (!bFlying)
	{
		return;
	}

	bFlying = false;
	SetComponentTickEnabled(false);
	FinishedDelegate.Unbind();

	OwnerCSKCharacter->GetMoveComponent()->EndLeapMovement();
}
