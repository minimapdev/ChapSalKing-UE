#include "Framework/CSKArenaBounds.h"

#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/StaticMesh.h"

ACSKArenaBounds::ACSKArenaBounds()
{
	PrimaryActorTick.bCanEverTick = false;

	BoundarySpline = CreateDefaultSubobject<USplineComponent>(TEXT("BoundarySpline"));
	SetRootComponent(BoundarySpline);

	// 경계는 고리다 — 열린 스플라인이면 벽에 구멍이 남는다
	BoundarySpline->SetClosedLoop(true);

	// 경계는 움직이지 않는다. 자식인 나무 클러스터를 Static으로 두려면 부모도 Static이어야 한다 —
	// Movable 부모에는 Static 자식이 붙지 않는다
	BoundarySpline->SetMobility(EComponentMobility::Static);
}

void ACSKArenaBounds::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RebuildBoundary();
}

void ACSKArenaBounds::BeginPlay()
{
	Super::BeginPlay();

	RebuildBoundary();
}

void ACSKArenaBounds::RebuildBoundary()
{
	for (UBoxComponent* Wall : WallSegments)
	{
		if (IsValid(Wall))
		{
			Wall->DestroyComponent();
		}
	}
	WallSegments.Reset();

	for (UHierarchicalInstancedStaticMeshComponent* Cluster : PropClusters)
	{
		if (IsValid(Cluster))
		{
			Cluster->DestroyComponent();
		}
	}
	PropClusters.Reset();

	// 점을 아직 안 찍었거나 한 점뿐인 상태 — 세울 것이 없다
	const float SplineLength = BoundarySpline->GetSplineLength();
	if (SplineLength <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	BuildWalls(SplineLength);
	BuildProps(SplineLength);
}

void ACSKArenaBounds::BuildWalls(float SplineLength)
{
	// 토막 길이를 나눠떨어지게 다시 잡는다. 그래야 마지막 토막이 짧게 남아 틈이 생기지 않는다
	const int32 SegmentCount = FMath::Max(1, FMath::RoundToInt(SplineLength / SegmentLength));
	const float Step = SplineLength / SegmentCount;

	for (int32 Index = 0; Index < SegmentCount; ++Index)
	{
		const float Distance = (Index + 0.5f) * Step;

		const FVector Location =
			BoundarySpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		// 벽은 곧게 서야 한다 — 스플라인이 언덕을 오르내려도 기울면 밑에 틈이 생긴다
		const FVector Direction =
			BoundarySpline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World)
				.GetSafeNormal2D();

		UBoxComponent* Wall = NewObject<UBoxComponent>(this);
		Wall->RegisterComponent();
		Wall->AttachToComponent(BoundarySpline, FAttachmentTransformRules::KeepRelativeTransform);
		Wall->SetWorldLocationAndRotation(
			Location + FVector(0.0f, 0.0f, 0.5f * WallHeight - WallSink),
			FRotationMatrix::MakeFromX(Direction).Rotator());

		// X를 반 두께만큼 늘려 이웃 토막과 겹친다 — 곡선이 꺾이는 곳의 쐐기 틈을 막는다
		Wall->SetBoxExtent(
			FVector(0.5f * Step + 0.5f * WallThickness, 0.5f * WallThickness, 0.5f * WallHeight));

		// 보이지 않는 벽이 그림자를 던지면 섬 안쪽에 정체불명의 긴 띠가 생긴다.
		// 높이가 2000이라 해가 비스듬할수록 그림자가 맵을 가로지른다
		Wall->SetCastShadow(false);

		// 에디터에서도 평소엔 숨긴다 — 토막이 수십 개라 켜두면 레벨이 안 보인다.
		// 액터를 선택하면 다시 나온다
		Wall->bDrawOnlyIfSelected = true;

		Wall->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Wall->SetCollisionObjectType(ECC_WorldStatic);
		Wall->SetCollisionResponseToAllChannels(ECR_Block);

		// 보이지 않는 벽이 조준 트레이스와 카메라까지 막으면 안 된다 —
		// 가장자리에서 카메라가 튀거나 조준선이 허공에 걸린다
		Wall->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		Wall->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

		// 네비메시를 여기서 끊는다. 봇이 경계를 따로 읽지 않아도 밖으로 못 나가는 근거
		Wall->SetCanEverAffectNavigation(true);

		WallSegments.Add(Wall);
	}
}

void ACSKArenaBounds::BuildProps(float SplineLength)
{
	for (UStaticMesh* Mesh : PropMeshes)
	{
		if (!Mesh)
		{
			continue;
		}

		UHierarchicalInstancedStaticMeshComponent* Cluster =
			NewObject<UHierarchicalInstancedStaticMeshComponent>(this);

		// 렌더 상태가 만들어지기 전에 정해야 하는 값들이라 RegisterComponent보다 먼저 둔다

		// 심어진 뒤로 움직이지 않는 배경이다 — 고정물로 취급받아 그림자·GI 캐시에 그대로 남는다
		Cluster->SetMobility(EComponentMobility::Static);

		Cluster->InstanceStartCullDistance = PropCullDistance.Min;
		Cluster->InstanceEndCullDistance = PropCullDistance.Max;

		Cluster->WorldPositionOffsetDisableDistance = PropSwayDistance;

		// 잎이 WPO로 흔들리면 VSM은 수백 그루의 그림자를 매 프레임 다시 그린다.
		// 그림자까지 흔들릴 필요는 없다 — 트랜스폼이 바뀔 때만 다시 그리게 한다
		Cluster->ShadowCacheInvalidationBehavior = EShadowCacheInvalidationBehavior::Rigid;

		Cluster->RegisterComponent();
		Cluster->AttachToComponent(BoundarySpline, FAttachmentTransformRules::KeepRelativeTransform);
		Cluster->SetStaticMesh(Mesh);

		// 충돌과 길막기는 벽이 전담한다. 나무마다 콜리전을 켜면 경계가 울퉁불퉁해지고
		// 네비메시도 나무 모양대로 파여 봇이 가장자리에서 낀다
		Cluster->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Cluster->SetCanEverAffectNavigation(false);

		PropClusters.Add(Cluster);
	}

	if (PropClusters.Num() == 0)
	{
		return;
	}

	// 씨앗을 고정한다 — 스플라인을 만질 때마다 숲 전체가 새로 뽑히면 손볼 수가 없다
	FRandomStream Random(PropRandomSeed);

	const int32 PropCount = FMath::Max(1, FMath::FloorToInt(SplineLength / PropSpacing));

	for (int32 Index = 0; Index < PropCount; ++Index)
	{
		const float Distance = Index * PropSpacing;

		const FVector Location =
			BoundarySpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		const FVector Right =
			BoundarySpline->GetRightVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World)
				.GetSafeNormal2D();

		const float Lateral = PropLateralOffset + Random.FRandRange(-PropLateralJitter, PropLateralJitter);
		const float Scale = Random.FRandRange(PropScaleRange.X, PropScaleRange.Y);

		FTransform PropTransform;
		PropTransform.SetLocation(Location + Right * Lateral);
		PropTransform.SetRotation(FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f).Quaternion());
		PropTransform.SetScale3D(FVector(Scale));

		UHierarchicalInstancedStaticMeshComponent* Cluster =
			PropClusters[Random.RandRange(0, PropClusters.Num() - 1)];
		Cluster->AddInstance(PropTransform, /*bWorldSpace*/ true);
	}
}
