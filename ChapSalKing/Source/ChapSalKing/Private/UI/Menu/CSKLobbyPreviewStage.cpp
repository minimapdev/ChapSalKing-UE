#include "UI/Menu/CSKLobbyPreviewStage.h"

#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "UI/Menu/CSKLobbyPreviewStand.h"
#include "UI/Menu/CSKMenuTypes.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// 엔진 기본 Plane은 법선이 로컬 +Z이고 한 변이 100uu다. 판을 쓰는 쪽은 프레임의 +X를
	// 시선 방향으로 잡으므로, 로컬에서 Pitch 90을 먹여야 법선이 -X를 향해 카메라를 마주본다.
	// 거기서 법선을 축으로 90도를 더 돌린다 — 이 메시의 UV 가로축이 로컬 X라서,
	// 그렇게 해야 텍스처 가로가 화면 가로와 맞는다.
	// 최종적으로 로컬 X가 화면 가로, 로컬 Y가 화면 세로다
	const FQuat PlaneFacingRotation =
		FRotator(90.0f, 0.0f, 0.0f).Quaternion() * FQuat(FVector::ZAxisVector, UE_HALF_PI);
	constexpr float PlaneMeshSize = 100.0f;
}

ACSKLobbyPreviewStage::ACSKLobbyPreviewStage()
{
	PrimaryActorTick.bCanEverTick = false;

	// 스탠드와 판을 월드 절대 좌표로 세우므로 이 액터 자신의 자리에는 아무 의미가 없다.
	// 프리뷰의 소유자이자, 로비 위젯이 월드에 닿는 유일한 통로일 뿐이다
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("StageRoot")));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	PlaneMesh = PlaneMeshFinder.Object;

	Backdrop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Backdrop"));
	Backdrop->SetupAttachment(GetRootComponent());
	Backdrop->SetStaticMesh(PlaneMesh);
	Backdrop->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 판은 Unlit이라 그림자를 받지도 않고, 던져봐야 뒤에 받을 것도 없다
	Backdrop->SetCastShadow(false);
}

void ACSKLobbyPreviewStage::BeginPlay()
{
	Super::BeginPlay();

	BuildStands();
}

void ACSKLobbyPreviewStage::BuildStands()
{
	UWorld* World = GetWorld();
	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	if (!World || !Settings)
	{
		return;
	}

	const FCSKLobbySettings& LobbySettings = Settings->LobbySettings;

	// UI 슬롯 격자와 같은 설정을 읽는다 — 칸 수도 칸 순서도 어긋날 수 없다
	const int32 StandCount = LobbySettings.Columns * LobbySettings.Rows;

	UMaterialInterface* CardMaterial = LobbySettings.SlotCardMaterial.LoadSynchronous();
	Backdrop->SetMaterial(0, LobbySettings.BackdropMaterial.LoadSynchronous());

	for (int32 Index = 0; Index < StandCount; ++Index)
	{
		// 지연 스폰이다. 소프트바디 컴포넌트가 BeginPlay에서 설정을 요구하므로
		// 폰에서 값을 옮겨 담기 전에 BeginPlay가 돌면 안 된다
		ACSKLobbyPreviewStand* Stand = World->SpawnActorDeferred<ACSKLobbyPreviewStand>(
			ACSKLobbyPreviewStand::StaticClass(), FTransform::Identity, this);

		Stand->InitFromPawnClass(Settings->BotSpawnSettings.BotPawnClass);
		Stand->SetSoftBodyData(LobbySettings.PreviewSoftBodyData);
		Stand->SetPreviewYaw(LobbySettings.PreviewYaw);
		Stand->FinishSpawning(FTransform::Identity);
		Stand->SetDough(nullptr);

		Stands.Add(Stand);

		UStaticMeshComponent* SlotCard = NewObject<UStaticMeshComponent>(this);
		SlotCard->SetupAttachment(GetRootComponent());
		SlotCard->SetStaticMesh(PlaneMesh);
		SlotCard->SetMaterial(0, CardMaterial);
		SlotCard->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SlotCard->SetCastShadow(false);
		SlotCard->RegisterComponent();

		SlotCards.Add(SlotCard);
	}

	UE_LOG(LogTemp, Log, TEXT("[CSK] LobbyPreviewStage: 스탠드 %d개"), Stands.Num());
}

void ACSKLobbyPreviewStage::ApplyRoster(const TArray<UDoughData*>& RosterDoughData)
{
	for (int32 Index = 0; Index < Stands.Num(); ++Index)
	{
		Stands[Index]->SetDough(
			RosterDoughData.IsValidIndex(Index) ? RosterDoughData[Index] : nullptr);
	}
}

void ACSKLobbyPreviewStage::SetStageVisible(bool bVisible)
{
	for (ACSKLobbyPreviewStand* Stand : Stands)
	{
		Stand->SetStageVisible(bVisible);
	}

	// 카드는 빈 자리에도 깔린다 — 반죽과 달리 명단을 타지 않는다.
	// 머티리얼이 없으면 흰 판이 되므로 그때는 계속 숨겨둔다
	for (UStaticMeshComponent* SlotCard : SlotCards)
	{
		SlotCard->SetVisibility(bVisible && SlotCard->GetMaterial(0) != nullptr);
	}

	Backdrop->SetVisibility(bVisible && Backdrop->GetMaterial(0) != nullptr);
}

void ACSKLobbyPreviewStage::AlignToSlots(
	APlayerController* PlayerController,
	const TArray<FCSKLobbySlotViewportRect>& SlotRects)
{
	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	if (!PlayerController || !Settings)
	{
		return;
	}

	const FCSKLobbySettings& LobbySettings = Settings->LobbySettings;
	const float PreviewDistance = LobbySettings.PreviewDistance;

	// 모든 것을 카메라 정면축 기준의 "같은 깊이 평면"에 올린다.
	// 역투영이 주는 시선 거리로 놓으면 화면 중앙에서 멀어질수록 깊이가 얕아져
	// 바깥쪽 칸의 반죽이 더 크게 보이고, 판은 화면과 평행하지 않아 기울어 보인다.
	// 같은 깊이에 두면 화면 어디서든 1픽셀이 같은 월드 길이가 된다
	const APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FRotator CameraRotation = CameraManager->GetCameraRotation();
	const FVector CameraForward = CameraRotation.Vector();

	const auto PointAtDepth = [&](const FVector2D& ScreenPosition, float Depth)
	{
		FVector RayOrigin, RayDirection;
		UGameplayStatics::DeprojectScreenToWorld(PlayerController, ScreenPosition, RayOrigin, RayDirection);

		// 광선 시작점은 카메라가 아니라 근평면 위다 — 깊이는 카메라에서 재야 한다
		const float OriginDepth = (RayOrigin - CameraLocation) | CameraForward;
		return RayOrigin + RayDirection * ((Depth - OriginDepth) / (RayDirection | CameraForward));
	};

	// 판은 화면과 평행해야 정확한 직사각형으로 찍힌다 — 방향은 칸마다가 아니라 카메라가 정한다
	const FQuat PlaneRotation = CameraRotation.Quaternion() * PlaneFacingRotation;

	for (int32 Index = 0; Index < Stands.Num(); ++Index)
	{
		if (!SlotRects.IsValidIndex(Index))
		{
			continue;
		}

		ACSKLobbyPreviewStand* Stand = Stands[Index];
		const float BaseHeight = Stand->GetBaseHeight();
		const float BaseWidth = Stand->GetBaseWidth();
		if (BaseHeight <= 0.0f || BaseWidth <= 0.0f)
		{
			continue;
		}

		const FCSKLobbySlotViewportRect& SlotRect = SlotRects[Index];
		const FVector2D HalfHeightOffset(0.0f, 0.5f * SlotRect.Height);
		const FVector2D HalfWidthOffset(0.5f * SlotRect.Width, 0.0f);

		const FVector SlotCenterWorld = PointAtDepth(SlotRect.Center, PreviewDistance);

		// 이 깊이에서 "칸 한 변 = 월드 몇 cm"인지가 배율과 카드 크기의 전부다.
		// 화각도 해상도도 DPI도 전부 역투영에 이미 들어 있어 따로 계산할 것이 없다
		const float SlotWorldHeight = FVector::Dist(
			PointAtDepth(SlotRect.Center - HalfHeightOffset, PreviewDistance),
			PointAtDepth(SlotRect.Center + HalfHeightOffset, PreviewDistance));
		const float SlotWorldWidth = FVector::Dist(
			PointAtDepth(SlotRect.Center - HalfWidthOffset, PreviewDistance),
			PointAtDepth(SlotRect.Center + HalfWidthOffset, PreviewDistance));

		// 세로·가로 중 빡빡한 쪽에 맞춘다. 세로만 보면 칸이 세로로 길 때
		// 반죽이 옆 칸을 파고든다
		const float Scale = LobbySettings.PreviewFillRatio * FMath::Min(
			SlotWorldHeight / BaseHeight,
			SlotWorldWidth / BaseWidth);

		// 각 반죽이 자기 시선 벡터를 정확히 마주보게 세운다. 카메라와 같은 회전으로 두면
		// 월드에서는 다 같은 쪽을 보지만 화면 가장자리 칸일수록 비스듬히 보여 옆면이 드러난다 —
		// 시선을 마주보게 해야 어느 칸이든 같은 면이 같은 각도로 찍힌다.
		// 액터 정면은 카메라를 등진 쪽이라 PreviewYaw의 의미(180이 정면)는 그대로다
		Stand->SetActorRotation((SlotCenterWorld - CameraLocation).Rotation());
		Stand->SetActorScale3D(FVector(Scale));

		// 메시가 캡슐 기준으로 액터 원점 아래에 달려 있다 — 그만큼 내려 잡아야
		// 반죽의 시각적 중심이 칸 중심에 온다. 스탠드가 카메라를 따라 기울었으므로
		// 월드 Z가 아니라 액터의 업 축(= 화면 세로)을 따라 내린다
		Stand->SetActorLocation(
			SlotCenterWorld - Stand->GetActorUpVector() * (Stand->GetBaseCenterZ() * Scale));

		// 카드는 반죽 뒤로 물러난다. 반죽이 클수록 더 물러나야 몸통이 판을 뚫지 않는다
		const float CardDepth = PreviewDistance
			+ 0.5f * BaseWidth * Scale
			+ LobbySettings.SlotCardDepthMargin;

		// 더 깊은 곳에서는 같은 칸이 원근으로 더 크게 보여야 화면에서 칸과 정확히 겹친다
		const float CardDepthRatio = CardDepth / PreviewDistance;

		UStaticMeshComponent* SlotCard = SlotCards[Index];
		SlotCard->SetWorldLocationAndRotation(PointAtDepth(SlotRect.Center, CardDepth), PlaneRotation);
		SlotCard->SetWorldScale3D(FVector(
			SlotWorldWidth * CardDepthRatio / PlaneMeshSize,
			SlotWorldHeight * CardDepthRatio / PlaneMeshSize,
			1.0f));
	}

	// 배경 판은 칸이 아니라 화면 전체를 덮는다 — 이게 레벨을 가린다
	int32 ViewportWidth = 0;
	int32 ViewportHeight = 0;
	PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);
	const FVector2D ViewportSize(ViewportWidth, ViewportHeight);

	const float BackdropDepth = PreviewDistance + LobbySettings.BackdropDistance;

	const FVector ScreenTopLeft = PointAtDepth(FVector2D::ZeroVector, BackdropDepth);
	const FVector ScreenBottomRight = PointAtDepth(ViewportSize, BackdropDepth);
	const FVector ScreenDiagonal = ScreenBottomRight - ScreenTopLeft;

	Backdrop->SetWorldLocationAndRotation(
		PointAtDepth(0.5f * ViewportSize, BackdropDepth), PlaneRotation);
	Backdrop->SetWorldScale3D(FVector(
		FMath::Abs(ScreenDiagonal | CameraRotation.RotateVector(FVector::YAxisVector)) / PlaneMeshSize,
		FMath::Abs(ScreenDiagonal | CameraRotation.RotateVector(FVector::ZAxisVector)) / PlaneMeshSize,
		1.0f));
}
