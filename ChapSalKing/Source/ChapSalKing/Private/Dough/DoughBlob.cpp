#include "Dough/DoughBlob.h"

#include "Dough/DoughBlobData.h"
#include "Dough/DoughData.h"
#include "Dough/DoughDataRegistry.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerScaleComponent.h"
#include "Player/Components/CSKPlayerStatusVFXComponent.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADoughBlob::ADoughBlob()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	BlobMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlobMesh"));
	SetRootComponent(BlobMesh);
	BlobMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlobMesh->SetCollisionObjectType(ECC_PhysicsBody);
	BlobMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BlobMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BlobMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	BlobMesh->SetCanEverAffectNavigation(false);
	BlobMesh->SetNotifyRigidBodyCollision(true);

	PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	PickupCollision->SetupAttachment(BlobMesh);
	PickupCollision->SetSphereRadius(80.0f);
	PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCollision->SetCollisionObjectType(ECC_WorldDynamic);
	PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

ADoughBlob* ADoughBlob::SpawnBlob(
	const UObject* WorldContext,
	const FVector& Location,
	EDoughCharacterType DoughType,
	float Grams,
	const FVector& LaunchDirection)
{
	if (Grams <= 0.0f)
	{
		return nullptr;
	}

	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(WorldContext);
	if (!Settings || !Settings->DoughBlobClass || !Settings->DoughBlobData)
	{
		return nullptr;
	}

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World)
	{
		return nullptr;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, Location);
	ADoughBlob* Blob = World->SpawnActorDeferred<ADoughBlob>(Settings->DoughBlobClass, SpawnTransform);
	if (!Blob)
	{
		return nullptr;
	}

	Blob->BlobData = Settings->DoughBlobData;
	Blob->DoughType = DoughType;
	Blob->Grams = Grams;
	Blob->FinishSpawning(SpawnTransform);

	Blob->Launch(LaunchDirection);
	return Blob;
}

void ADoughBlob::BeginPlay()
{
	Super::BeginPlay();

	if (!BlobData)
	{
		Destroy();
		return;
	}

	PickupCollision->SetSphereRadius(BlobData->PickupRadius);
	PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &ADoughBlob::OnPickupOverlap);
	BlobMesh->OnComponentHit.AddDynamic(this, &ADoughBlob::HandleMeshHit);

	ApplyVisual();

	SetPickupEnabled(false);
	if (UWorld* World = GetWorld())
	{
		if (BlobData->PickupLockout > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				PickupLockoutTimerHandle, this,
				&ADoughBlob::FinishPickupLockout, BlobData->PickupLockout, false);
		}
		else
		{
			SetPickupEnabled(true);
		}
	}
}

void ADoughBlob::ApplyVisual()
{
	if (BlobData->BlobMesh)
	{
		BlobMesh->SetStaticMesh(BlobData->BlobMesh);
	}

	const float BlobScale = FMath::Clamp(
		Grams / BlobData->GramsPerBlobScale,
		BlobData->MinBlobScale,
		BlobData->MaxBlobScale);
	RoundScale = FVector(BlobScale);
	SetActorScale3D(RoundScale);

	if (BlobData->BlobMaterial)
	{
		BlobMesh->SetMaterial(0, BlobData->BlobMaterial);
	}

	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	UDoughDataRegistry* Registry = Settings ? Settings->DoughDataRegistry : nullptr;
	UDoughData* Dough = Registry ? Registry->FindByType(DoughType) : nullptr;
	if (!Dough)
	{
		return;
	}

	if (UMaterialInstanceDynamic* DynMaterial = BlobMesh->CreateDynamicMaterialInstance(0))
	{
		DynMaterial->SetVectorParameterValue(BlobData->ColorParameterName, Dough->BodyColor);
	}
}

void ADoughBlob::Launch(const FVector& LaunchDirection)
{
	BlobMesh->BodyInstance.bLockXRotation = true;
	BlobMesh->BodyInstance.bLockYRotation = true;
	BlobMesh->BodyInstance.bLockZRotation = true;
	BlobMesh->SetSimulatePhysics(true);
	BlobMesh->BodyInstance.SetDOFLock(EDOFMode::SixDOF);
	BlobMesh->SetLinearDamping(BlobData->LinearDamping);

	FVector Direction = LaunchDirection;
	Direction.Z = 0.0f;
	if (Direction.IsNearlyZero())
	{
		Direction = FMath::VRand();
		Direction.Z = 0.0f;
	}
	Direction = Direction.GetSafeNormal();

	const FVector Impulse = Direction * BlobData->ImpulseStrength + FVector(0.0f, 0.0f, BlobData->ImpulseUp);
	BlobMesh->AddImpulse(Impulse, NAME_None, true);
}

void ADoughBlob::HandleMeshHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (bLanded)
	{
		return;
	}

	if (Hit.ImpactNormal.Z < BlobData->GroundNormalZThreshold)
	{
		return;
	}

	bLanded = true;

	BlobMesh->SetSimulatePhysics(false);

	LandBottomZ = BlobMesh->Bounds.Origin.Z - BlobMesh->Bounds.BoxExtent.Z;
	LandPivotOffset = GetActorLocation().Z - LandBottomZ;

	SquashTargetScale = FVector(
		RoundScale.X * BlobData->LandSquashXY,
		RoundScale.Y * BlobData->LandSquashXY,
		RoundScale.Z * BlobData->LandSquashZ);

	if (BlobData->LandSquashDuration <= 0.0f)
	{
		SetActorScale3D(SquashTargetScale);
		FVector Loc = GetActorLocation();
		Loc.Z = LandBottomZ + LandPivotOffset * (SquashTargetScale.Z / RoundScale.Z);
		SetActorLocation(Loc);
		return;
	}

	SquashElapsed = 0.0f;
	bSquashing = true;
	SetActorTickEnabled(true);
}

void ADoughBlob::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bSquashing)
	{
		return;
	}

	SquashElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(SquashElapsed / BlobData->LandSquashDuration, 0.0f, 1.0f);
	const float Eased = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f);

	const FVector Scale = FMath::Lerp(RoundScale, SquashTargetScale, Eased);
	SetActorScale3D(Scale);

	FVector Loc = GetActorLocation();
	Loc.Z = LandBottomZ + LandPivotOffset * (Scale.Z / RoundScale.Z);
	SetActorLocation(Loc);

	if (Alpha >= 1.0f)
	{
		bSquashing = false;
		SetActorTickEnabled(false);
	}
}

void ADoughBlob::SetPickupEnabled(bool bEnabled)
{
	PickupCollision->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void ADoughBlob::FinishPickupLockout()
{
	SetPickupEnabled(true);
}

void ADoughBlob::OnPickupOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ACSKPlayerCharacter* Character = Cast<ACSKPlayerCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	UCSKPlayerScaleComponent* ScaleComponent = Character->GetScaleComponent();
	if (!ScaleComponent)
	{
		return;
	}

	ScaleComponent->AddMass(Grams);
	if (UCSKPlayerStatusVFXComponent* VFXComponent = Character->GetStatusVFXComponent())
	{
		VFXComponent->PlayOneShot(ECSKStatusVisualType::DoughPickup);
	}
	Destroy();
}
