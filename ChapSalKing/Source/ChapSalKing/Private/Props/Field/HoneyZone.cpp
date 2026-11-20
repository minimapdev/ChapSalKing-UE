#include "Props/Field/HoneyZone.h"
#include "Props/Field/HoneyZoneData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerMoveComponent.h"
#include "Player/Components/CSKPlayerIngredientComponent.h"

#include "Components/BoxComponent.h"

static const FName HoneyZoneSpeedFactorKey(TEXT("HoneyZone"));

// 장판 판정 박스의 반높이 (반경과 무관하게 낮게 유지 — 바닥에 붙은 판 느낌)
static constexpr float HoneyZoneHalfHeight = 100.0f;

AHoneyZone::AHoneyZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneCollision"));
	ZoneCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ZoneCollision->SetCollisionObjectType(ECC_WorldDynamic);
	ZoneCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	ZoneCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ZoneCollision->SetGenerateOverlapEvents(true);
	ZoneCollision->OnComponentBeginOverlap.AddDynamic(this, &AHoneyZone::HandleBeginOverlap);
	ZoneCollision->OnComponentEndOverlap.AddDynamic(this, &AHoneyZone::HandleEndOverlap);
	RootComponent = ZoneCollision;
}

void AHoneyZone::BeginPlay()
{
	Super::BeginPlay();

	if (!ZoneData)
	{
		UE_LOG(LogTemp, Warning, TEXT("HoneyZone: ZoneData is not assigned."));
		Destroy();
		return;
	}

	const float Radius = AreaRadiusOverride > 0.0f ? AreaRadiusOverride : ZoneData->ZoneRadius;
	ZoneCollision->SetBoxExtent(FVector(Radius, Radius, HoneyZoneHalfHeight));
	SetLifeSpan(ZoneData->LifeSeconds);

	BP_OnZoneSpawned(Radius);
}

void AHoneyZone::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	ACSKPlayerCharacter* Character = Cast<ACSKPlayerCharacter>(OtherActor);
	if (!Character || Occupants.Contains(Character))
	{
		return;
	}

	Occupants.Add(Character);

	if (UCSKPlayerIngredientComponent* IngredientComp = Character->GetIngredientComponent())
	{
		IngredientComp->OnIngredientStateChanged.AddUniqueDynamic(
			this, &AHoneyZone::HandleOccupantIngredientChanged);
	}

	ApplySpeedFactorTo(*Character);
}

void AHoneyZone::HandleEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	ACSKPlayerCharacter* Character = Cast<ACSKPlayerCharacter>(OtherActor);
	if (!Character || !Occupants.Contains(Character))
	{
		return;
	}

	Occupants.Remove(Character);

	if (UCSKPlayerIngredientComponent* IngredientComp = Character->GetIngredientComponent())
	{
		IngredientComp->OnIngredientStateChanged.RemoveDynamic(
			this, &AHoneyZone::HandleOccupantIngredientChanged);
	}

	TArray<AActor*> OverlappingZones;
	Character->GetOverlappingActors(OverlappingZones, AHoneyZone::StaticClass());
	OverlappingZones.Remove(this);
	if (OverlappingZones.Num() > 0)
	{
		return;
	}

	if (UCSKPlayerMoveComponent* Move = Character->GetMoveComponent())
	{
		Move->ClearSpeedFactor(HoneyZoneSpeedFactorKey);
	}
}

void AHoneyZone::HandleOccupantIngredientChanged(UIngredientData* Ingredient, float GaugeRatio)
{
	for (ACSKPlayerCharacter* Occupant : Occupants)
	{
		if (Occupant)
		{
			ApplySpeedFactorTo(*Occupant);
		}
	}
}

void AHoneyZone::ApplySpeedFactorTo(ACSKPlayerCharacter& Character) const
{
	UCSKPlayerMoveComponent* Move = Character.GetMoveComponent();
	if (!Move)
	{
		return;
	}

	// 같은 계열(꿀 재료 반죽, 꿀떡 킹)은 가속
	const bool bBoosted = ZoneData->ZoneAffinity != ECSKAffinity::None
		&& Character.GetAffinity() == ZoneData->ZoneAffinity;
	Move->SetSpeedFactor(
		HoneyZoneSpeedFactorKey,
		bBoosted ? ZoneData->HoneyHolderSpeedFactor : ZoneData->SlowFactor
	);
}
