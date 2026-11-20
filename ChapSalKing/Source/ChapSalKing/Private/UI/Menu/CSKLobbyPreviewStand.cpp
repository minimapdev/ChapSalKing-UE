#include "UI/Menu/CSKLobbyPreviewStand.h"

#include "Dough/DoughData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerSoftBodyComponent.h"
#include "Player/Components/CSKPlayerVisualComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"

ACSKLobbyPreviewStand::ACSKLobbyPreviewStand()
{
	PrimaryActorTick.bCanEverTick = false;

	StandRoot = CreateDefaultSubobject<USceneComponent>(TEXT("StandRoot"));
	SetRootComponent(StandRoot);

	// 메시를 루트로 두면 아래 SetRelativeTransform이 액터 월드 위치를 덮어써
	// 스탠드가 자리에서 통째로 이탈한다
	DoughMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DoughMesh"));
	DoughMesh->SetupAttachment(StandRoot);
	DoughMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 로비 밖에서는 숨어 있다 — 안 보이는 동안 포즈까지 돌릴 이유가 없다
	DoughMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	// 대기 모션을 인게임과 같은 경로로 돌린다. 설정값은 InitFromPawnClass가 폰에서 가져온다
	SoftBodyComponent = CreateDefaultSubobject<UCSKPlayerSoftBodyComponent>(TEXT("SoftBodyComponent"));

	// 로비 화면에 들어오기 전까지는 숨어 있다 — 레벨에 실제로 서 있으므로
	// 숨기지 않으면 타이틀 화면 카메라에도 그대로 잡힌다
	SetActorHiddenInGame(true);
}

void ACSKLobbyPreviewStand::InitFromPawnClass(TSubclassOf<ACSKPlayerCharacter> PawnClass)
{
	const ACSKPlayerCharacter* PawnDefault =
		PawnClass ? PawnClass->GetDefaultObject<ACSKPlayerCharacter>() : nullptr;
	USkeletalMeshComponent* SourceMesh = PawnDefault ? PawnDefault->GetMesh() : nullptr;
	if (!SourceMesh)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CSK] LobbyPreviewStand: 폰 클래스에서 스켈레탈 메시를 찾지 못했습니다"));
		return;
	}

	DoughMesh->SetSkeletalMesh(SourceMesh->GetSkeletalMeshAsset());
	DoughMesh->SetAnimInstanceClass(SourceMesh->GetAnimClass());

	// 메시만 가져오면 BP가 컴포넌트에 덮어쓴 머티리얼이 빠진다 —
	// 반죽색·얼굴 파라미터가 없는 기본 머티리얼로 떨어져 흰 덩어리가 된다
	for (int32 MaterialIndex = 0; MaterialIndex < SourceMesh->GetNumMaterials(); ++MaterialIndex)
	{
		DoughMesh->SetMaterial(MaterialIndex, SourceMesh->GetMaterial(MaterialIndex));
	}

	// 프리뷰는 칸 카드 위에 얹힌 그림이다 — 그림자가 지면 카드에 얼룩이 생기고
	// 칸마다 방향이 달라 보인다. 음영은 라이팅만으로 충분하다
	DoughMesh->SetCastShadow(false);

	// 캡슐 기준으로 잡힌 메시 오프셋까지 가져와야 반죽이 칸 중심에 온다
	DoughMesh->SetRelativeTransform(SourceMesh->GetRelativeTransform());
	MeshBaseRotation = DoughMesh->GetRelativeRotation();

	// 칸 높이에 맞출 배율을 내려면 스케일 1일 때의 크기를 알아야 한다.
	// 애님 포즈가 아니라 레퍼런스 포즈 바운드를 쓴다 — 대기 모션을 따라
	// 반죽 크기가 매 프레임 출렁이면 안 된다
	const FBoxSphereBounds MeshBounds = DoughMesh->GetSkeletalMeshAsset()->GetBounds();
	const FVector MeshScale = DoughMesh->GetRelativeScale3D();
	BaseHeight = MeshBounds.BoxExtent.Z * 2.0f * MeshScale.Z;
	BaseCenterZ = DoughMesh->GetRelativeLocation().Z + MeshBounds.Origin.Z * MeshScale.Z;

	// 메시가 요로 돌아가 있어 X·Y 중 어느 쪽이 화면 가로가 될지 정해져 있지 않다 —
	// 넓은 쪽으로 잡아야 어느 각도에서도 칸을 넘지 않는다
	BaseWidth = 2.0f * FMath::Max(
		MeshBounds.BoxExtent.X * MeshScale.X,
		MeshBounds.BoxExtent.Y * MeshScale.Y);
}

void ACSKLobbyPreviewStand::SetSoftBodyData(UDoughSoftBodyData* SoftBodyData)
{
	SoftBodyComponent->SoftBodyData = SoftBodyData;
}

void ACSKLobbyPreviewStand::SetPreviewYaw(float Yaw)
{
	DoughMesh->SetRelativeRotation(MeshBaseRotation + FRotator(0.0f, Yaw, 0.0f));
}

void ACSKLobbyPreviewStand::SetDough(UDoughData* DoughData)
{
	bHasDough = DoughData != nullptr;
	UpdateVisibility();

	if (!DoughData)
	{
		return;
	}

	// 캐릭터·동상과 같은 경로 — 프리뷰가 인게임 외형과 어긋나지 않는다
	UCSKPlayerVisualComponent::ApplyDoughVisualToMesh(DoughData, DoughMesh);
}

void ACSKLobbyPreviewStand::SetStageVisible(bool bVisible)
{
	bStageVisible = bVisible;
	UpdateVisibility();
}

void ACSKLobbyPreviewStand::UpdateVisibility()
{
	SetActorHiddenInGame(!(bHasDough && bStageVisible));
}
