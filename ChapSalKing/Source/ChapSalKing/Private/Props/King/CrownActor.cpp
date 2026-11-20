#include "Props/King/CrownActor.h"

#include "Player/Components/CSKCarryVisualFollowComponent.h"
#include "Framework/CSKGameInstance.h"
#include "Player/CSKPlayerCharacter.h"
#include "Framework/CSKProjectSettingsData.h"
#include "Animation/AnimSequence.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Props/King/CrownData.h"
#include "TimerManager.h"

namespace
{
float GetAnimationDuration(const UAnimSequence* Animation, float PlayRate)
{
	if (!Animation)
	{
		return 0.0f;
	}

	const float EffectivePlayRate = FMath::Abs(PlayRate);
	return EffectivePlayRate > UE_KINDA_SMALL_NUMBER
		? Animation->GetPlayLength() / EffectivePlayRate
		: Animation->GetPlayLength();
}
}

ACrownActor::ACrownActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CrownMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CrownMesh"));
	CrownMesh->SetupAttachment(SceneRoot);
	CrownMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CrownMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);

	CarryVisualFollowComponent = CreateDefaultSubobject<UCSKCarryVisualFollowComponent>(TEXT("CarryVisualFollowComponent"));
}

void ACrownActor::BeginPlay()
{
	Super::BeginPlay();

	InitCrownData();
	ApplyCrownData();
}

void ACrownActor::InitializeCrown(ACSKPlayerCharacter* InOwnerCharacter)
{
	if (!InOwnerCharacter)
	{
		return;
	}

	InitCrownData();

	if (!CrownData || !CarryVisualFollowComponent)
	{
		return;
	}

	ApplyCrownData();

	// 디스폰 애니 진행 중 재사용되면 타이머가 새 왕관을 파괴하므로 반드시 해제
	GetWorldTimerManager().ClearTimer(DespawnAnimationTimerHandle);

	CrownOwnerCharacter = InOwnerCharacter;
	bIsRemoving = false;

	CarryVisualFollowComponent->StartFollow(
		CrownOwnerCharacter,
		CrownMesh,
		CrownData->CarryVisualFollowSettings
	);

	PlaySpawnAnimation();
}

void ACrownActor::RemoveCrown()
{
	if (bIsRemoving)
	{
		return;
	}

	bIsRemoving = true;

	PlayDespawnAnimation();
}

void ACrownActor::InitCrownData()
{
	if (const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this))
	{
		CrownData = Settings->CrownData;
	}
}

void ACrownActor::ApplyCrownData()
{
	if (!CrownData || !CrownMesh)
	{
		return;
	}

	if (CrownData->CrownMesh)
	{
		CrownMesh->SetSkeletalMesh(CrownData->CrownMesh);
	}
}

void ACrownActor::PlayCrownAnimation(UAnimSequence* Animation, bool bLooping, float PlayRate)
{
	if (!CrownMesh || !Animation)
	{
		return;
	}

	CrownMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	CrownMesh->PlayAnimation(Animation, bLooping);
	CrownMesh->SetPlayRate(PlayRate);

	const float StartPosition = PlayRate < 0.0f
		? FMath::Max(Animation->GetPlayLength() - UE_KINDA_SMALL_NUMBER, 0.0f)
		: 0.0f;

	CrownMesh->SetPosition(StartPosition, false);
}

void ACrownActor::PlaySpawnAnimation()
{
	if (!CrownData || !CrownMesh)
	{
		return;
	}

	if (!CrownData->CrownTransitionAnimation)
	{
		return;
	}

	constexpr float SpawnPlayRate = -1.0f;
	PlayCrownAnimation(CrownData->CrownTransitionAnimation, false, SpawnPlayRate);
}

void ACrownActor::PlayDespawnAnimation()
{
	if (!CrownData || !CrownMesh)
	{
		FinishRemoveCrown();
		return;
	}

	if (!CrownData->CrownTransitionAnimation)
	{
		FinishRemoveCrown();
		return;
	}

	constexpr float DespawnPlayRate = 1.0f;
	PlayCrownAnimation(CrownData->CrownTransitionAnimation, false, DespawnPlayRate);

	GetWorldTimerManager().SetTimer(
		DespawnAnimationTimerHandle,
		this,
		&ACrownActor::FinishRemoveCrown,
		GetAnimationDuration(CrownData->CrownTransitionAnimation, DespawnPlayRate),
		false
	);
}

void ACrownActor::FinishRemoveCrown()
{
	GetWorldTimerManager().ClearTimer(DespawnAnimationTimerHandle);

	if (CarryVisualFollowComponent)
	{
		CarryVisualFollowComponent->StopFollow();
	}

	CrownOwnerCharacter = nullptr;
	Destroy();
}
