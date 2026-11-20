#include "Props/Field/HoneyDrop.h"
#include "Props/Field/HoneyDropData.h"
#include "Props/Field/HoneyZone.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AHoneyDrop::AHoneyDrop()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetNotifyRigidBodyCollision(true);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	MeshComponent->BodyInstance.bLockXRotation = true;
	MeshComponent->BodyInstance.bLockYRotation = true;
	MeshComponent->BodyInstance.bLockZRotation = true;
	RootComponent = MeshComponent;
}

void AHoneyDrop::BeginPlay()
{
	Super::BeginPlay();

	if (!DropData || !DropData->ZoneClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HoneyDrop] DropData/ZoneClass가 비어있어 파괴합니다."));
		Destroy();
		return;
	}

	MeshComponent->OnComponentHit.AddDynamic(this, &AHoneyDrop::HandleHit);
	SetLifeSpan(DropData->MaxLifeSeconds);
}

void AHoneyDrop::HandleHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	if (Hit.ImpactNormal.Z < DropData->GroundNormalZThreshold)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AHoneyZone>(DropData->ZoneClass, Hit.ImpactPoint, FRotator::ZeroRotator, SpawnParams);

	BP_OnDropLanded(Hit.ImpactPoint);
	Destroy();
}
