#include "Props/Mortar/PestleItem.h"

#include "Framework/CSKGameInstance.h"
#include "Player/Components/CSKCarryVisualFollowComponent.h"
#include "Player/CSKPlayerCharacter.h"
#include "Framework/CSKProjectSettingsData.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Props/Mortar/PestleItemData.h"
#include "Props/Mortar/MortarZone.h"
#include "Props/Mortar/PestleSpawnPoint.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "EngineUtils.h"

APestleItem::APestleItem()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트
	PestleBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PestleBody"));
	SetRootComponent(PestleBody);
	PestleBody->SetHiddenInGame(true);
	PestleBody->SetSimulatePhysics(false);
	PestleBody->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PestleBody->SetCollisionObjectType(ECC_PhysicsBody);
	PestleBody->SetCollisionResponseToAllChannels(ECR_Ignore);
	PestleBody->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	PestleBody->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	PestleBody->SetCanEverAffectNavigation(false);

	// 비주얼 메시
	PestleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PestleMesh"));
	PestleMesh->SetupAttachment(PestleBody);
	PestleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 픽업
	PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	PickupCollision->SetupAttachment(PestleBody);
	PickupCollision->SetSphereRadius(80.0f);
	PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCollision->SetCollisionObjectType(ECC_WorldDynamic);
	PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	CarryVisualFollowComponent = CreateDefaultSubobject<UCSKCarryVisualFollowComponent>(TEXT("CarryVisualFollowComponent"));

	// 위치 알림 발광 (운반 중에도 보이도록 비주얼 메시에 부착)
	BeaconEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BeaconEffectComponent"));
	BeaconEffectComponent->SetupAttachment(PestleMesh);
	BeaconEffectComponent->bAutoActivate = false;
	// 절구공이가 회전하며 날아가도 이펙트는 항상 월드 Up 방향 유지
	BeaconEffectComponent->SetUsingAbsoluteRotation(true);
}

void APestleItem::BeginPlay()
{
	Super::BeginPlay();

	InitPestleData();
	ReturnLocation = GetActorLocation();
	PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &APestleItem::OnPickupOverlap);

	if (PestleData->BeaconEffect)
	{
		BeaconEffectComponent->SetAsset(PestleData->BeaconEffect);
	}
	SetBeaconActive(true);
}

void APestleItem::StashIntoPouch()
{
	// 절구에 꽂혀 있던 공이라면 먼저 뽑아낸다 — 사라지는 자리를 BP가 연출할 수 있게 알린다
	if (bPlantedInMortar)
	{
		bPlantedInMortar = false;
		BP_OnPestleUnplanted(GetActorLocation());
	}

	if (CarryVisualFollowComponent)
	{
		CarryVisualFollowComponent->StopFollow();
	}

	if (CarrierCharacter)
	{
		CarrierCharacter->ClearCarriedPestleItem(this);
		CarrierCharacter = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PickupLockoutTimerHandle);
	}

	bStashed = true;

	PestleBody->SetSimulatePhysics(false);
	PestleBody->SetPhysicsLinearVelocity(FVector::ZeroVector);
	PestleBody->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	SetPickupEnabled(false);
	SetBeaconActive(false);

	// 액터는 매치 내내 하나로 유지되므로 파괴하지 않는다. 주머니가 열릴 때까지 숨어 있을 뿐이다
	SetActorHiddenInGame(true);
}

void APestleItem::RevealAt(const FVector& Location)
{
	bStashed = false;
	ReturnLocation = Location;

	SetActorLocationAndRotation(
		Location, FQuat::Identity, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
	SetPickupEnabled(true);
	SetBeaconActive(true);

	BP_OnPestleRevealed(Location);
}

void APestleItem::OnPickupOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ACSKPlayerCharacter* Character = Cast<ACSKPlayerCharacter>(OtherActor);
	if (!CanBePickedUpBy(Character))
	{
		return;
	}

	CarrierCharacter = Character;
	CarrierCharacter->SetCarriedPestleItem(this);

	SetPickupEnabled(false);

	PestleBody->SetSimulatePhysics(false);

	CarryVisualFollowComponent->StartFollow(
		CarrierCharacter,
		PestleMesh,
		PestleData->CarryVisualFollowSettings
	);

	TArray<AActor*> OverlappingZones;
	Character->GetOverlappingActors(OverlappingZones, AMortarZone::StaticClass());
	for (AActor* Zone : OverlappingZones)
	{
		CastChecked<AMortarZone>(Zone)->NotifyPestlePickedUpInside(Character);
	}
}

bool APestleItem::CanBePickedUpBy(const ACSKPlayerCharacter* Character) const
{
	return Character != nullptr && CarrierCharacter == nullptr;
}

bool APestleItem::IsCarriedBy(const ACSKPlayerCharacter* Character) const
{
	return CarrierCharacter != nullptr && CarrierCharacter == Character;
}

void APestleItem::InitPestleData()
{
	if (const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this))
	{
		PestleData = Settings->PestleItemData;
	}
	checkf(PestleData, TEXT("PestleItemData가 설정되지 않았습니다 (ProjectSettings 또는 인스턴스)."));
}

void APestleItem::DropFromCarrier(const FVector& PreferredDirection)
{
	if (!CarrierCharacter)
	{
		return;
	}

	ACSKPlayerCharacter* PreviousCarrier = CarrierCharacter;

	if (CarryVisualFollowComponent)
	{
		CarryVisualFollowComponent->StopFollow();
	}

	PreviousCarrier->ClearCarriedPestleItem(this);

	CarrierCharacter = nullptr;

	LaunchWithPhysics(GetRandomDropDirection(PreferredDirection), PestleData->LaunchSettings);
}

void APestleItem::LaunchWithPhysics(const FVector& Direction, const FCSKPhysicsLaunchSettings& Launch)
{
	SetPickupEnabled(false);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PickupLockoutTimerHandle, this,
			&APestleItem::FinishPickupLockout, Launch.PickupLockout, false);
	}

	PestleBody->SetSimulatePhysics(true);
	const FVector Impulse = Direction * Launch.ImpulseStrength + FVector(0.0f, 0.0f, Launch.ImpulseUp);
	PestleBody->AddImpulse(Impulse, NAME_None, true);
	PestleBody->AddAngularImpulseInDegrees(FMath::VRand() * Launch.SpinStrength, NAME_None, true);
}

void APestleItem::PlantInMortar(const FTransform& PlantTransform)
{
	if (CarryVisualFollowComponent)
	{
		CarryVisualFollowComponent->StopFollow();
	}

	if (CarrierCharacter)
	{
		CarrierCharacter->ClearCarriedPestleItem(this);
		CarrierCharacter = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PickupLockoutTimerHandle);
	}

	bPlantedInMortar = true;
	SetBeaconActive(false);
	SetPickupEnabled(false);
	PestleBody->SetSimulatePhysics(false);
	SetActorLocationAndRotation(
		PlantTransform.GetLocation(), PlantTransform.GetRotation(),
		false, nullptr, ETeleportType::TeleportPhysics);

	ReturnLocation = PlantTransform.GetLocation();
}

void APestleItem::PrepareForMortarSequence()
{
	if (CarryVisualFollowComponent)
	{
		CarryVisualFollowComponent->StopFollow();
	}

	if (CarrierCharacter)
	{
		CarrierCharacter->ClearCarriedPestleItem(this);
		CarrierCharacter = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PickupLockoutTimerHandle);
	}

	SetBeaconActive(false);
	SetPickupEnabled(false);
	PestleBody->SetSimulatePhysics(false);
	PestleBody->SetPhysicsLinearVelocity(FVector::ZeroVector);
	PestleBody->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
}

FVector APestleItem::GetRandomDropDirection(const FVector& PreferredDirection) const
{
	FVector Direction = PreferredDirection;
	Direction.Z = 0.0f;

	if (Direction.IsNearlyZero())
	{
		Direction = FMath::VRand();
		Direction.Z = 0.0f;
	}

	Direction = Direction.GetSafeNormal();

	const float RandomYaw = FMath::RandRange(-70.0f, 70.0f);
	return Direction.RotateAngleAxis(RandomYaw, FVector::UpVector).GetSafeNormal();
}

void APestleItem::FinishPickupLockout()
{
	SetPickupEnabled(true);
}

void APestleItem::SetPickupEnabled(bool bEnabled)
{
	if (!PickupCollision)
	{
		return;
	}

	PickupCollision->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void APestleItem::SetBeaconActive(bool bActive)
{
	if (bActive)
	{
		BeaconEffectComponent->Activate();
		PestleMesh->SetOverlayMaterial(PestleData->BeaconOverlayMaterial);
	}
	else
	{
		BeaconEffectComponent->Deactivate();
		PestleMesh->SetOverlayMaterial(nullptr);
	}
}

void APestleItem::FellOutOfWorld(const UDamageType& DmgType)
{
	PestleBody->SetPhysicsLinearVelocity(FVector::ZeroVector);
	PestleBody->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

	TArray<APestleSpawnPoint*> Points;
	for (TActorIterator<APestleSpawnPoint> It(GetWorld()); It; ++It)
	{
		Points.Add(*It);
	}

	const FVector Fallback = Points.Num() > 0
		? Points[FMath::RandRange(0, Points.Num() - 1)]->GetActorLocation()
		: ReturnLocation;
	SetActorLocation(Fallback, false, nullptr, ETeleportType::TeleportPhysics);
}
