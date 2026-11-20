#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
class UStaticMesh;
class UMaterialInterface;

#include "SnakeData.generated.h"

// 뱀의 행동 모드. 사양: Docs/ChapSalKing_Snake.md 3.2
UENUM(BlueprintType)
enum class ESnakeMode : uint8
{
	// 현재 반경을 유지하며 계속 돈다
	Orbit,
	// 반대편에 점을 찍고 코어를 관통한다
	Cross,
	// 돌면서 반경을 줄여 안으로 감겨든다
	Coil,
	// 땅에 들어가 가장 가까운 반죽 쪽으로 가서 솟아오른다
	Lurk
};

// 모드 하나의 튜닝값. 사양 6.3
USTRUCT(BlueprintType)
struct FSnakeModeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Mode")
	ESnakeMode Mode = ESnakeMode::Orbit;

	// BaseSpeed에 곱한다. 횡단·잠복만 1을 넘는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Mode", meta = (ClampMin = "0.1"))
	float SpeedMultiplier = 1.0f;

	// 모드 추첨 가중치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Mode", meta = (ClampMin = "0.0"))
	float SelectionWeight = 40.0f;

	// 지속시간 기반 모드(순회·잠복)에서 쓴다.
	// 횡단은 목표 도달로, 똬리는 회전수로 끝나므로 무시된다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Mode", meta = (ClampMin = "0.0", Units = "s"))
	float MinDuration = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Mode", meta = (ClampMin = "0.0", Units = "s"))
	float MaxDuration = 20.0f;

	// 이 모드로 들어갈 때 제자리에서 예고하는 시간. 사양 3.4
	// 위험한 모드(횡단·잠복)만 1.5초를 갖는다 — 순회·똬리는 느리고 경로가 보이므로 0이다.
	// **횡단·잠복의 값을 줄이지 말 것.** 이게 공정성의 전부다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Mode", meta = (ClampMin = "0.0", Units = "s"))
	float TelegraphDuration = 0.0f;
};

// 뱀의 모든 수치. 사양: Docs/ChapSalKing_Snake.md 7장
UCLASS(BlueprintType)
class CHAPSALKING_API USnakeData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	USnakeData();

	// 기준 속도. 플레이어 WalkSpeed(800)보다 반드시 낮게 —
	// 넘으면 뱀이 추격자가 되고 킹과 역할이 겹친다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Move", meta = (ClampMin = "50.0", Units = "cm/s"))
	float BaseSpeed = 600.0f;

	// 뱀다움의 핵심. 크면 화살표처럼 꺾이고, 작으면 목표 주위를 맴돈다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Move", meta = (ClampMin = "5.0"))
	float MaxTurnRateDeg = 90.0f;

	// 이 안에 들어오면 다음 목표점으로 넘어간다.
	// 선회 반경(속도 / 라디안 선회율)보다 커야 한다. 작으면 목표를 영원히 돌기만 한다 —
	// 기본값 기준 최대 선회 반경은 횡단의 960 / 1.571 = 611
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Move", meta = (ClampMin = "50.0", Units = "cm"))
	float TargetReachRadius = 700.0f;

	// 머리가 위아래로 기울어지는 최대 각도. 파고들 때 땅을 향해 코를 박고,
	// 솟을 때 하늘을 향한다. 0이면 자세가 수평으로 고정돼 "푹 꺼지는" 것처럼 보인다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Move", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float MaxPitchDeg = 60.0f;

	// 기울기가 목표를 따라가는 속도. 낮으면 굼뜨고, 높으면 지형 요철에 머리가 떤다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Move", meta = (ClampMin = "0.1"))
	float PitchInterpSpeed = 5.0f;

	// 지형 위로 띄우는 높이. 머리와 마디 모두 이 높이에 놓인다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Move", meta = (ClampMin = "0.0", Units = "cm"))
	float HeightOffset = 120.0f;

	// 지형 높이를 찾는 수직 트레이스의 위아래 길이
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Move", meta = (ClampMin = "100.0", Units = "cm"))
	float GroundTraceHeight = 3000.0f;

	// 아레나 중심은 월드 원점이다 (Docs/ChapSalKing_LandscapeSpec.md 2장).
	// 이 밖은 오목 벽이라 뱀이 타고 오르면 우스꽝스러워진다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Arena", meta = (ClampMin = "500.0", Units = "cm"))
	float MaxOrbitRadius = 4400.0f;

	// 이보다 안쪽에서는 순회하지 않는다. 반경이 작으면 한 스텝 앞 목표점이
	// TargetReachRadius 안에 들어와 제자리에서 돌게 된다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Arena", meta = (ClampMin = "500.0", Units = "cm"))
	float MinOrbitRadius = 2000.0f;

	// 순회 모드가 시작될 때 진행 방향을 뒤집을 확률.
	// 1이면 매번 무작위로 뒤집혀 제자리 U턴이 잦고, 그때마다 머리가 자기 몸통을 관통한다.
	// 낮게 두면 반전이 드물어지고, 그래야 "방향 반전 예고"(3.4)도 의미가 생긴다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Orbit", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OrbitReverseChance = 0.2f;

	// 순회·똬리가 한 번에 찍는 각도. 크면 원이 다각형이 된다.
	// 목표점 사이의 현 길이 = 2 x r x sin(이 각도/2) 이고, 이게 TargetReachRadius보다
	// 커야 한다 — 그래서 반경 하한(MinOrbitRadius, CoilMinRadius)이 따라온다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Orbit", meta = (ClampMin = "5.0", ClampMax = "90.0"))
	float OrbitStepDeg = 30.0f;

	// 횡단 목표 방위의 흔들림. 180°에서 이만큼 좌우로 벌어진다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Cross", meta = (ClampMin = "0.0", ClampMax = "80.0"))
	float CrossAngleSpreadDeg = 40.0f;

	// 횡단 목표 반경. MaxOrbitRadius를 넘으면 머리가 클램프에 걸려
	// 목표에 영원히 닿지 못하므로 코드에서 함께 클램프한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Cross", meta = (ClampMin = "500.0", Units = "cm"))
	float CrossRadiusMin = 3500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Cross", meta = (ClampMin = "500.0", Units = "cm"))
	float CrossRadiusMax = 4300.0f;

	// 한 바퀴 도는 동안 반경이 줄어드는 양
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Coil", meta = (ClampMin = "50.0", Units = "cm"))
	float CoilStepIn = 1400.0f;

	// 여기 닿으면 감기를 멈추고 같은 폭으로 풀면서 나온다.
	// OrbitStepDeg 30° 기준 현 길이가 TargetReachRadius(700)를 넘으려면 1,400 이상이어야 한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Coil", meta = (ClampMin = "500.0", Units = "cm"))
	float CoilMinRadius = 1600.0f;

	// 똬리가 도는 바퀴 수. **이 값이 똬리의 지속시간을 결정한다** —
	// 한 바퀴는 2 x pi x r / (BaseSpeed x 속도배수)초, r=4000 기준 약 46초다.
	// 0.75면 약 35초. 1을 넘기면 한 모드가 매치의 1/4를 잡아먹는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Coil", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float CoilRevolutions = 0.75f;

	// 잠복이 겨냥하는 반죽을 모드 시작 시점에 한 번만 고른다.
	// 계속 따라가게 만들지 말 것 — 뱀이 의도를 가진 추격자로 읽히면 킹과 역할이 겹친다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Lurk", meta = (ClampMin = "0.0", Units = "cm"))
	float LurkTargetReachRadius = 400.0f;

	// 잠수했을 때 지형 아래로 내려가는 깊이. 지형이 뱀을 가린다 —
	// 메시를 숨기거나 스케일을 0으로 만들지 않는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Submerge", meta = (ClampMin = "50.0", Units = "cm"))
	float SubmergeDepth = 400.0f;

	// 순회·똬리 중 이 간격마다 땅에 들어갔다 나온다. 몸통에 뚫리는 구멍의 빈도다.
	// 길면 맵이 자주 반토막 난다 (사양 3.5). 횡단·잠복 중에는 일어나지 않는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Submerge", meta = (ClampMin = "0.5", Units = "s"))
	float SubmergeIntervalMin = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Submerge", meta = (ClampMin = "0.5", Units = "s"))
	float SubmergeIntervalMax = 14.0f;

	// 한 번 잠긴 채로 있는 시간. 구멍의 크기다 — 이 시간 x 속도만큼의 몸통이 사라진다.
	// 들어가고 나오는 전환 시간이 이 안에 포함되므로 그만큼 넉넉히 준다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Submerge", meta = (ClampMin = "0.1", Units = "s"))
	float SubmergeDurationMin = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Submerge", meta = (ClampMin = "0.1", Units = "s"))
	float SubmergeDurationMax = 5.0f;

	// 지상 높이에서 SubmergeDepth까지 내려가는 데 걸리는 시간. 나올 때도 같다.
	// 0이면 한 프레임에 순간이동해서 "파고든다"가 아니라 "깜빡인다"로 보인다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Submerge", meta = (ClampMin = "0.0", Units = "s"))
	float SubmergeTransitionDuration = 0.35f;

	// 땅속에 있는 동안 지면에 흙 자국을 남기는 주기.
	// 이게 없으면 뱀이 사라진 것으로 보이고, 솟아오르는 순간이 무예고 기습이 된다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Submerge", meta = (ClampMin = "0.02", Units = "s"))
	float SubmergeTrailInterval = 0.18f;

	// 땅속에서 솟기 몇 초 전에 BP_OnSurfaceTelegraph를 부를지. 0이면 예고하지 않는다.
	// 사양 1.2 — 못 읽는 위험은 그냥 불공정한 사망이다. 잠수는 예고 없이는 피할 방법이 없다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Submerge", meta = (ClampMin = "0.0", Units = "s"))
	float SurfaceTelegraphDuration = 1.0f;

	// 예고 중 속도 배수. 0이면 완전히 멈춘다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Telegraph", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TelegraphSpeedMultiplier = 0.15f;

	// 예고 중 선회율. 갈 방향을 정면으로 노려봐야 하므로 평소보다 빠르다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Telegraph", meta = (ClampMin = "5.0"))
	float TelegraphTurnRateDeg = 180.0f;

	// 몸 길이 = HeadSegmentGap + SegmentSpacing x (SegmentCount - 1).
	// 플레이 지름보다 살짝 길어야 횡단 한 번이 맵을 두 쪽으로 자른다 (사양 3.6).
	// 반대로 몸 길이가 뱀이 그리는 원의 둘레보다 길면 머리가 자기 꼬리를 문다 —
	// 최소 반경(MinOrbitRadius / CoilMinRadius)이 몸 길이 / (2 x pi)보다 커야 한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (ClampMin = "1", ClampMax = "200"))
	int32 SegmentCount = 25;

	// 머리와 첫 마디 사이 간격. 마디끼리의 간격과 따로 둔다 —
	// 머리 메시가 마디보다 크므로 SegmentSpacing을 그대로 쓰면 첫 마디가 머리 안에 파묻힌다.
	// 기준: 머리 뒤쪽 반경 + 첫 마디 반경 이상
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (ClampMin = "0.0", Units = "cm"))
	float HeadSegmentGap = 600.0f;

	// 가장 굵은 마디(머리 쪽) 기준 간격. **꼬리로 갈수록 굵기에 비례해 좁아진다** —
	// 간격이 고정이면 TailScaleRatio로 가늘어진 꼬리에서 메시가 뚝뚝 끊긴다.
	// 메시 지름보다 작아야 몸통이 이어져 보인다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (ClampMin = "10.0", Units = "cm"))
	float SegmentSpacing = 400.0f;

	// 몸통 굵기. 엔진 기본 구(지름 100uu)를 쓰면 5가 몸통 반지름 250uu다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (ClampMin = "0.01"))
	float SegmentMeshScale = 5.0f;

	// 꼬리 끝 마디의 스케일 비율. 1이면 굵기가 일정하고, 낮출수록 뒤로 갈수록 가늘어진다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float TailScaleRatio = 0.5f;

	// 머리 판정 구의 반지름. **월드 단위이고 HeadMeshScale의 영향을 받지 않는다.**
	// 스켈레탈 메시의 바운드에서 뽑지 않는다 — 그 값은 PhysicsAsset의 AABB와
	// Bounds Extension이 섞인 것이라 눈에 보이는 크기와 다르다. 눈으로 맞추는 값이다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (ClampMin = "10.0", Units = "cm"))
	float MouthRadius = 300.0f;

	// 판정 구를 머리 앞쪽으로 미는 거리. 입이 앞에 달린 머리는 뒤통수로 삼키면 안 된다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (ClampMin = "0.0", Units = "cm"))
	float MouthForwardOffset = 0.0f;

	// 머리 크기. BP에서 따로 만지지 않도록 여기서 준다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (ClampMin = "0.01"))
	float HeadMeshScale = 6.0f;

	// 마디와 둔덕은 런타임에 만드는 컴포넌트라 BP에서 꽂을 수 없다. 여기서 준다.
	// 인스턴스가 아니라 개별 StaticMeshComponent라 머티리얼에
	// 'Used with Instanced Static Meshes' 플래그가 필요 없다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body")
	TObjectPtr<UStaticMesh> SegmentMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body")
	TObjectPtr<UMaterialInterface> SegmentMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Mound")
	TObjectPtr<UStaticMesh> MoundMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Mound")
	TObjectPtr<UMaterialInterface> MoundMaterial;

	// --- 포식. 사양 5장 ---

	// 삼킬 때 뜯어내는 몸집 비율. 킹은 0.3(EatMassStealFraction)이다.
	// **1.0으로 두지 말 것** — 뱀이 킹보다 무서워지면 게임의 중심축이 밀린다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Bite", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BiteMassRatio = 0.4f;

	// 뜯은 몸집을 몇 덩어리로 나눠 뿌리나. 1이면 한 번에 회수돼 긴장이 없고,
	// 많으면 줍는 게 노동이 된다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Bite", meta = (ClampMin = "1", ClampMax = "20"))
	int32 BlobCount = 4;

	// 덩어리를 뿌리는 좌우 부채꼴의 반각. 앞뒤로 뿌리면 뱀 몸통 아래 깔려서 줍지 못한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Bite", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float BlobSideSpreadDeg = 45.0f;

	// 입으로 빨려 들어가는 시간. 이 동안은 머리에 붙어 있다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Bite", meta = (ClampMin = "0.0", Units = "s"))
	float SwallowDuration = 0.2f;

	// 삼킨 자리에서 자취를 거슬러 꼬리 쪽으로 밀려가는 거리.
	// 시간이 아니라 거리라서 횡단(1.6배속)에도 항상 같은 자리에 뱉어진다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Bite", meta = (ClampMin = "0.0", Units = "cm"))
	float SwallowTravelDistance = 1200.0f;

	// 삼킨 것이 지나가는 마디가 부풀어 오르는 배율. 밖에서 혹으로 보인다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Bite", meta = (ClampMin = "1.0", ClampMax = "4.0"))
	float SwallowBulgeScale = 1.6f;

	// 뱉을 때 몸통 옆으로 밀어내는 힘. 자취 위에 그대로 뱉으면 몸통 안에 낀다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Bite", meta = (ClampMin = "0.0"))
	float EjectSideStrength = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Bite", meta = (ClampMin = "0.0"))
	float EjectUpStrength = 400.0f;

	// 뱉어진 뒤 다시 먹히지 않는 시간. **뱀에게만 통한다** — 킹 회피 수단이 되면 안 된다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Bite", meta = (ClampMin = "0.0", Units = "s"))
	float BiteImmunityDuration = 2.0f;

	// 매치 시작 직후 이 시간 동안 뱀은 **땅속에 있다.** 지하 자국만 흘리다가 이 시간이 지나면 솟아오른다.
	// 못 무는 이유가 규칙이 아니라 눈에 보이는 상태가 된다 — 판정은 CanBite가 따로 보지 않는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Bite", meta = (ClampMin = "0.0", Units = "s"))
	float StartGraceDuration = 10.0f;

	// 잠긴 마디 위 지면에 올리는 둔덕의 가로 배율. 마디 굵기에 곱한다.
	// 이 이랑이 "땅속을 지나간다"를 보여주는 주역이다 — 먼지로는 크기가 안 읽힌다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Mound", meta = (ClampMin = "0.1"))
	float MoundWidthScale = 1.8f;

	// 둔덕의 높이 배율. 낮게 눌러야 지면이 솟은 것처럼 보인다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Mound", meta = (ClampMin = "0.05"))
	float MoundHeightScale = 0.3f;

	// 둔덕을 지면 아래로 파묻는 비율(자기 높이 기준). 0이면 반구가 통째로 드러나 과자처럼 보인다.
	// 묻을수록 이웃과 겹쳐 하나의 이랑으로 읽힌다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Mound", meta = (ClampMin = "0.0", ClampMax = "0.9"))
	float MoundSinkRatio = 0.35f;

	// 자취를 기록하는 간격. 촘촘할수록 곡선이 매끄럽고 버퍼가 길어진다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (ClampMin = "5.0", Units = "cm"))
	float TrailSampleDistance = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Mode")
	TArray<FSnakeModeSettings> Modes;
};
