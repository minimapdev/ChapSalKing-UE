#include "Props/Pouch/LuckyPouch.h"

#include "Camera/CSKCameraShakeStatics.h"
#include "Camera/CSKCameraShakeTags.h"
#include "Combat/CSKCombatStatics.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "Ingredient/IngredientData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerIngredientComponent.h"
#include "Props/Mortar/PestleItem.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

namespace
{
	const FCSKPouchDropSettings* GetDropSettings(const UObject* WorldContext)
	{
		const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(WorldContext);
		return Settings ? &Settings->PouchDropSettings : nullptr;
	}
}

ALuckyPouch::ALuckyPouch()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	PouchBody = CreateDefaultSubobject<USphereComponent>(TEXT("PouchBody"));
	SetRootComponent(PouchBody);
	PouchBody->SetSphereRadius(80.0f);
	PouchBody->SetCollisionObjectType(ECC_WorldDynamic);
	PouchBody->SetCollisionResponseToAllChannels(ECR_Block);
	// 폰만 겹침이다 — 주머니가 플레이어를 밀어내면 안 되고, 닿는 순간이 곧 획득이다
	PouchBody->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PouchBody->SetGenerateOverlapEvents(true);

	// 물리를 쓰지 않는다. 튕겨서 굴러가면 그림자 마커가 거짓말이 되기 때문이다 —
	// 마커를 보고 달려간 자리에 주머니가 없으면 예고의 의미가 사라진다.
	// 주머니는 마커가 찍힌 자리에 그대로 얹힌다
	PouchBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PouchBody->SetSimulatePhysics(false);
	PouchBody->SetEnableGravity(false);

	PouchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PouchMesh"));
	PouchMesh->SetupAttachment(PouchBody);
	PouchMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ALuckyPouch::BeginPlay()
{
	Super::BeginPlay();

	PouchBody->OnComponentBeginOverlap.AddDynamic(this, &ALuckyPouch::OnPickupOverlap);
}

void ALuckyPouch::StartDrop(
	const FVector& InLandingLocation,
	UIngredientData* InIngredient,
	bool bInIsBomb,
	APestleItem* InPestle)
{
	const FCSKPouchDropSettings* DropSettings = GetDropSettings(this);
	if (!DropSettings)
	{
		return;
	}

	Ingredient = InIngredient;
	bIsBomb = bInIsBomb;
	StashedPestle = InPestle;

	LandingLocation = InLandingLocation;

	// 구의 중심을 지면에 놓으면 절반이 땅에 박힌 채로 물리가 켜진다 — 반경만큼 띄워 얹는다
	RestLocation = LandingLocation + FVector(0.0f, 0.0f, PouchBody->GetScaledSphereRadius());

	DropHeight = DropSettings->DropHeight;
	FallDuration = DropSettings->FallDuration;
	FallElapsed = 0.0f;
	Phase = EPouchPhase::Shadow;

	// 그림자 단계에는 주머니가 보이지 않는다 — 땅의 마커만 보고 달려가는 시간이다
	SetActorLocation(RestLocation + FVector(0.0f, 0.0f, DropHeight), false, nullptr, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(true);
	PouchBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (DropSettings->ShadowEffect)
	{
		ShadowEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			DropSettings->ShadowEffect,
			LandingLocation,
			FRotator::ZeroRotator,
			FVector(DropSettings->ShadowEffectScale),
			/*bAutoDestroy*/ false,
			/*bAutoActivate*/ true);
	}

	BP_OnShadowShown(LandingLocation, DropSettings->ShadowLeadTime);

	if (DropSettings->ShadowLeadTime > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			PhaseTimerHandle, this, &ALuckyPouch::BeginFall, DropSettings->ShadowLeadTime, false);
	}
	else
	{
		BeginFall();
	}
}

void ALuckyPouch::BeginFall()
{
	Phase = EPouchPhase::Falling;
	FallElapsed = 0.0f;

	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
}

void ALuckyPouch::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Phase != EPouchPhase::Falling)
	{
		SetActorTickEnabled(false);
		return;
	}

	FallElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(FallElapsed / FMath::Max(FallDuration, UE_SMALL_NUMBER), 0.0f, 1.0f);
	SetActorLocation(
		RestLocation + FVector(0.0f, 0.0f, DropHeight * (1.0f - Alpha)),
		false, nullptr, ETeleportType::TeleportPhysics);

	if (Alpha >= 1.0f)
	{
		Land();
	}
}

void ALuckyPouch::Land()
{
	const FCSKPouchDropSettings* DropSettings = GetDropSettings(this);
	if (!DropSettings)
	{
		return;
	}

	Phase = EPouchPhase::Landed;
	SetActorTickEnabled(false);

	if (ShadowEffectComponent)
	{
		ShadowEffectComponent->Deactivate();
		ShadowEffectComponent = nullptr;
	}

	// 마커가 찍힌 그 자리에 그대로 얹힌다. 물리를 켜지 않으므로 굴러가지도, 지면을 뚫지도 않는다.
	// 픽업만 필요하므로 콜리전은 쿼리뿐이다
	SetActorLocation(RestLocation, false, nullptr, ETeleportType::TeleportPhysics);
	PouchBody->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 물리가 없으니 "쿵 떨어졌다"는 느낌은 전적으로 이 연출이 만든다.
	// 이펙트는 지면(LandingLocation)에서 터져야 흙먼지가 땅에 붙어 보인다
	if (DropSettings->LandingEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, DropSettings->LandingEffect, LandingLocation, FRotator::ZeroRotator,
			FVector(DropSettings->LandingEffectScale), /*bAutoDestroy*/ true, /*bAutoActivate*/ true);
	}

	if (DropSettings->LandingCameraShakeScale > 0.0f)
	{
		UCSKCameraShakeStatics::PlayCameraShakeAtLocation(
			this, CSKCameraShakeTags::Pouch_Landing, LandingLocation,
			DropSettings->LandingCameraShakeScale);
	}

	// 절구공이 주머니는 아무도 안 열면 매치 루프가 멈춘다 — 스스로 터진다.
	// 그 밖의 주머니는 회수되지 않으면 사라진다. 맵에 무한정 쌓이지 않게
	if (IsPestlePouch())
	{
		GetWorldTimerManager().SetTimer(
			PhaseTimerHandle, this, &ALuckyPouch::HandleAutoOpen,
			DropSettings->PestlePouchAutoOpenDelay, false);
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			PhaseTimerHandle, this, &ALuckyPouch::HandleLifetimeExpired,
			DropSettings->PouchLifetime, false);
	}

	BP_OnLanded();
}

void ALuckyPouch::OnPickupOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bOpened || Phase != EPouchPhase::Landed)
	{
		return;
	}

	if (ACSKPlayerCharacter* Character = Cast<ACSKPlayerCharacter>(OtherActor))
	{
		Open(Character);
	}
}

void ALuckyPouch::HandleAutoOpen()
{
	// 아무도 열지 않았다. 절구공이는 어디에 있든 때가 되면 나온다
	Open(nullptr);
}

void ALuckyPouch::HandleLifetimeExpired()
{
	Destroy();
}

void ALuckyPouch::Open(ACSKPlayerCharacter* Opener)
{
	if (bOpened)
	{
		return;
	}

	bOpened = true;
	PouchBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);

	const FCSKPouchDropSettings* DropSettings = GetDropSettings(this);
	const FVector OpenLocation = GetActorLocation();

	if (DropSettings && DropSettings->OpenEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, DropSettings->OpenEffect, OpenLocation, FRotator::ZeroRotator,
			FVector(DropSettings->OpenEffectScale), /*bAutoDestroy*/ true, /*bAutoActivate*/ true);
	}

	if (StashedPestle)
	{
		// 절구공이는 매치 내내 단일 액터다 — 여기서 만들지 않고 숨겨 둔 것을 드러낸다
		StashedPestle->RevealAt(OpenLocation);
	}
	else if (bIsBomb)
	{
		Explode();
	}
	else if (Ingredient && Opener && !Opener->IsChapSalKing())
	{
		// 킹은 재료를 쓸 수 없다. 밟으면 주머니만 터지고 내용물은 사라진다 —
		// 킹이 얻는 것은 없지만 남에게 못 가게 막을 수는 있다
		if (UCSKPlayerIngredientComponent* IngredientComponent = Opener->GetIngredientComponent())
		{
			IngredientComponent->LoadIngredient(Ingredient);
		}
	}

	BP_OnOpened(StashedPestle != nullptr, bIsBomb);
	Destroy();
}

void ALuckyPouch::Explode()
{
	UWorld* World = GetWorld();
	const FCSKPouchDropSettings* DropSettings = GetDropSettings(this);
	if (!World || !DropSettings)
	{
		return;
	}

	const FCSKPouchBombSettings& Bomb = DropSettings->BombSettings;
	const FVector Origin = GetActorLocation();

	if (Bomb.ExplosionEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, Bomb.ExplosionEffect, Origin, FRotator::ZeroRotator,
			FVector(Bomb.ExplosionEffectScale), /*bAutoDestroy*/ true, /*bAutoActivate*/ true);
	}

	// 위치 기반이므로 멀리 있는 사람은 약하게 흔들린다 — 어디서 터졌는지가 화면으로 전해진다
	UCSKCameraShakeStatics::PlayCameraShakeAtLocation(
		this, CSKCameraShakeTags::Pouch_BombExplosion, Origin, Bomb.CameraShakeScale);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(
		Overlaps,
		Origin,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Bomb.Radius));

	TSet<AActor*> Processed;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Victim = Overlap.GetActor();
		if (!Victim || Processed.Contains(Victim))
		{
			continue;
		}
		Processed.Add(Victim);

		FVector Direction = Victim->GetActorLocation() - Origin;
		Direction.Z = 0.0f;
		if (!Direction.Normalize())
		{
			Direction = FVector::ForwardVector;
		}

		// 공격자를 두지 않는다. 연 사람도 반경 안에 있으면 똑같이 맞아야
		// "남이 먼저 열게 유도"가 성립한다
		FCSKCombatStatics::ApplyKnockbackHit(
			nullptr,
			Victim,
			Direction,
			Bomb.KnockbackStrength,
			Bomb.KnockbackUpwardStrength,
			Bomb.TargetScaleResistancePower,
			Bomb.DropFraction);
	}
}
