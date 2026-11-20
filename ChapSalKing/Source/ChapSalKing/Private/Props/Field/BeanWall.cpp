#include "Props/Field/BeanWall.h"
#include "Props/Field/BeanWallData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Combat/CSKCombatStatics.h"
#include "Framework/CSKGameInstance.h"

#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ABeanWall::ABeanWall()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	WallVisuals = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallVisuals"));
	WallVisuals->SetupAttachment(RootComponent);
	WallVisuals->SetMobility(EComponentMobility::Movable);
	WallVisuals->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WallVisuals->SetGenerateOverlapEvents(false);
	WallVisuals->SetVisibility(true);
	WallVisuals->SetHiddenInGame(false);
}

void ABeanWall::BeginPlay()
{
	Super::BeginPlay();

	if (!WallData)
	{
		UE_LOG(LogTemp, Warning, TEXT("BeanWall: WallData is not assigned."));
		Destroy();
		return;
	}

	if (AreaRadiusOverride <= 0.0f)
	{
		Destroy();
		return;
	}

	const float WallRadius = AreaRadiusOverride;
	const bool bEnableCollisionImmediately = WallData->CollisionEnableDelay <= 0.0f;

	BuildWallVisuals(WallRadius, WallData->WallHeight, WallData->SegmentCount);
	if (VisualTargetTransforms.IsEmpty())
	{
		Destroy();
		return;
	}

	const auto CreateCollisionSegment =
		[this, bEnableCollisionImmediately](
			const FVector& SegmentLocation,
			const FRotator& SegmentRotation,
			const FVector& BlockExtent)
	{
		const FVector PadExtent =
			BlockExtent + FVector(WallData->BounceTriggerPadding, WallData->BounceTriggerPadding, 0.0f);

		UBoxComponent* Block = NewObject<UBoxComponent>(this);
		Block->RegisterComponent();
		Block->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		Block->SetRelativeLocationAndRotation(SegmentLocation, SegmentRotation);
		Block->SetBoxExtent(BlockExtent);
		Block->SetCollisionEnabled(
			bEnableCollisionImmediately ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		Block->SetCollisionObjectType(ECC_WorldStatic);
		Block->SetCollisionResponseToAllChannels(ECR_Block);
		BlockingSegments.Add(Block);

		UBoxComponent* Pad = NewObject<UBoxComponent>(this);
		Pad->OnComponentBeginOverlap.AddDynamic(this, &ABeanWall::HandlePadOverlap);
		Pad->RegisterComponent();
		Pad->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		Pad->SetRelativeLocationAndRotation(SegmentLocation, SegmentRotation);
		Pad->SetBoxExtent(PadExtent);
		Pad->SetCollisionEnabled(
			bEnableCollisionImmediately ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		Pad->SetCollisionObjectType(ECC_WorldDynamic);
		Pad->SetCollisionResponseToAllChannels(ECR_Ignore);
		Pad->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Pad->SetGenerateOverlapEvents(bEnableCollisionImmediately);
		BouncePads.Add(Pad);

		if (UCSKGameInstance::IsDrawDebugEnabled(this))
		{
			DrawDebugBox(
				GetWorld(),
				GetActorLocation() + SegmentLocation,
				BlockExtent,
				SegmentRotation.Quaternion(),
				FColor::Red, false, WallData->LifeSeconds, 0, 4.0f);
		}
	};

	const UStaticMesh* VisualMesh = WallVisuals->GetStaticMesh();
	const FBoxSphereBounds MeshBounds = VisualMesh->GetBounds();
	const FVector SafePadding(
		FMath::Max(VisualCollisionPadding.X, 0.0f),
		FMath::Max(VisualCollisionPadding.Y, 0.0f),
		FMath::Max(VisualCollisionPadding.Z, 0.0f));

	for (const FTransform& VisualTransform : VisualTargetTransforms)
	{
		const FVector AbsoluteScale = VisualTransform.GetScale3D().GetAbs();
		const FVector BlockExtent = MeshBounds.BoxExtent * AbsoluteScale + SafePadding;
		const FVector BoundsCenter = VisualTransform.TransformPosition(MeshBounds.Origin);
		CreateCollisionSegment(
			BoundsCenter,
			VisualTransform.GetRotation().Rotator(),
			BlockExtent);
	}

	bWallCollisionEnabled = bEnableCollisionImmediately;
	EffectiveLifeSeconds = FMath::Max(
		WallData->LifeSeconds,
		RiseDuration + RiseStagger + SinkDuration);
	SpawnTimeSeconds = GetWorld()->GetTimeSeconds();

	BP_OnWallSpawned(WallRadius, WallData->WallHeight, BlockingSegments.Num());
}

void ABeanWall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!WallData || EffectiveLifeSeconds <= 0.0f)
	{
		return;
	}

	const float AgeSeconds = GetWorld()->GetTimeSeconds() - SpawnTimeSeconds;

	if (!bWallCollisionEnabled
		&& !bDespawning
		&& AgeSeconds >= WallData->CollisionEnableDelay)
	{
		SetWallCollisionEnabled(true);
	}

	const float SinkStartTime = FMath::Max(0.0f, EffectiveLifeSeconds - SinkDuration);
	if (!bDespawning && AgeSeconds >= SinkStartTime)
	{
		bDespawning = true;
		SetWallCollisionEnabled(false);
		BP_OnWallDespawning();
	}

	UpdateWallVisuals(AgeSeconds);

	if (AgeSeconds >= EffectiveLifeSeconds)
	{
		Destroy();
	}
}

void ABeanWall::BuildWallVisuals(float WallRadius, float WallHeight, int32 CollisionSegmentCount)
{
	WallVisuals->ClearInstances();
	VisualTargetTransforms.Reset();
	VisualRiseDepths.Reset();
	VisualRiseDelays.Reset();

	const UStaticMesh* VisualMesh = WallVisuals->GetStaticMesh();
	if (!VisualMesh)
	{
		return;
	}

	const int32 InstanceCount = FMath::Clamp(CollisionSegmentCount, 1, 64);

	const FBoxSphereBounds MeshBounds = VisualMesh->GetBounds();
	const float MeshHeight = FMath::Max(MeshBounds.BoxExtent.Z * 2.0f, 1.0f);
	const float HeightFitScale = bFitVisualHeightToWallHeight ? WallHeight / MeshHeight : 1.0f;
	const float AngleStep = 360.0f / InstanceCount;
	FRandomStream RandomStream(1847);

	VisualTargetTransforms.Reserve(InstanceCount);
	VisualRiseDepths.Reserve(InstanceCount);
	VisualRiseDelays.Reserve(InstanceCount);

	for (int32 Index = 0; Index < InstanceCount; ++Index)
	{
		const float AngleDeg = Index * AngleStep;
		const float RandomScale = RandomStream.FRandRange(
			FMath::Min(VisualRandomScaleMin, VisualRandomScaleMax),
			FMath::Max(VisualRandomScaleMin, VisualRandomScaleMax));
		const FVector InstanceScale =
			VisualScaleMultiplier * HeightFitScale * RandomScale;

		const float ScaledOriginZ = MeshBounds.Origin.Z * InstanceScale.Z;
		const float ScaledHalfHeight = MeshBounds.BoxExtent.Z * FMath::Abs(InstanceScale.Z);
		const float TargetPivotZ = VisualGroundOffset - ScaledOriginZ + ScaledHalfHeight;
		const float RiseDepth = FMath::Max(ScaledHalfHeight * 2.0f * UndergroundDepthMultiplier, 1.0f);

		const FRotator RadialRotation(0.0f, AngleDeg, 0.0f);
		const FRotator MeshRotation(
			0.0f,
			AngleDeg + VisualYawOffset + RandomStream.FRandRange(-VisualRandomYaw, VisualRandomYaw),
			0.0f);
		const FVector TargetLocation =
			RadialRotation.Vector() * (WallRadius + VisualRadiusOffset)
			+ FVector(0.0f, 0.0f, TargetPivotZ);

		const FTransform TargetTransform(MeshRotation, TargetLocation, InstanceScale);
		FTransform StartTransform = TargetTransform;
		StartTransform.AddToTranslation(FVector(0.0f, 0.0f, -RiseDepth));

		VisualTargetTransforms.Add(TargetTransform);
		VisualRiseDepths.Add(RiseDepth);
		VisualRiseDelays.Add(RandomStream.FRandRange(0.0f, FMath::Max(RiseStagger, 0.0f)));
		WallVisuals->AddInstance(StartTransform);
	}
}

void ABeanWall::UpdateWallVisuals(float AgeSeconds)
{
	if (VisualTargetTransforms.IsEmpty())
	{
		return;
	}

	const float SafeRiseDuration = FMath::Max(RiseDuration, KINDA_SMALL_NUMBER);
	const float SafeSinkDuration = FMath::Max(SinkDuration, KINDA_SMALL_NUMBER);
	const float SinkStartTime = FMath::Max(0.0f, EffectiveLifeSeconds - SafeSinkDuration);
	const bool bRiseFinished = AgeSeconds >= RiseDuration + RiseStagger;
	const bool bSinkNotStarted = AgeSeconds < SinkStartTime;
	if (bRiseFinished && bSinkNotStarted)
	{
		return;
	}

	for (int32 Index = 0; Index < VisualTargetTransforms.Num(); ++Index)
	{
		float VisibleAlpha = 1.0f;
		if (AgeSeconds < SinkStartTime)
		{
			const float RiseAlpha = FMath::Clamp(
				(AgeSeconds - VisualRiseDelays[Index]) / SafeRiseDuration,
				0.0f,
				1.0f);
			VisibleAlpha = 1.0f - FMath::Pow(1.0f - RiseAlpha, 3.0f);
		}
		else
		{
			const float SinkAlpha = FMath::Clamp(
				(AgeSeconds - SinkStartTime) / SafeSinkDuration,
				0.0f,
				1.0f);
			VisibleAlpha = 1.0f - FMath::InterpEaseIn(0.0f, 1.0f, SinkAlpha, 2.0f);
		}

		FTransform InstanceTransform = VisualTargetTransforms[Index];
		InstanceTransform.AddToTranslation(
			FVector(0.0f, 0.0f, -VisualRiseDepths[Index] * (1.0f - VisibleAlpha)));

		WallVisuals->UpdateInstanceTransform(
			Index,
			InstanceTransform,
			false,
			Index == VisualTargetTransforms.Num() - 1,
			true);
	}
}

void ABeanWall::SetWallCollisionEnabled(bool bEnabled)
{
	bWallCollisionEnabled = bEnabled;

	for (UBoxComponent* Block : BlockingSegments)
	{
		if (IsValid(Block))
		{
			Block->SetCollisionEnabled(
				bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		}
	}

	for (UBoxComponent* Pad : BouncePads)
	{
		if (IsValid(Pad))
		{
			Pad->SetGenerateOverlapEvents(bEnabled);
			Pad->SetCollisionEnabled(
				bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		}
	}
}

void ABeanWall::HandlePadOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	ACSKPlayerCharacter* Character = Cast<ACSKPlayerCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (const float* LastTime = LastBounceTimes.Find(OtherActor))
	{
		if (Now - *LastTime < WallData->BounceCooldownSeconds)
		{
			return;
		}
	}
	LastBounceTimes.Add(OtherActor, Now);

	// 중심(킹 착지점) 방향으로 밀어냄 — 끌어당김
	FVector Direction = GetActorLocation() - Character->GetActorLocation();
	Direction.Z = 0.0f;
	if (!Direction.Normalize())
	{
		return;
	}

	FCSKCombatStatics::ApplyKnockbackHit(
		GetOwner(),
		Character,
		Direction,
		WallData->BounceStrength,
		WallData->BounceUpwardStrength,
		WallData->TargetScaleResistancePower,
		0.0f
	);
}
