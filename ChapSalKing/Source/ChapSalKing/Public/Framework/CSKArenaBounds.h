#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSKArenaBounds.generated.h"

class USplineComponent;
class UBoxComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;

/**
 * 아레나 경계. 매치 레벨에 하나만 놓고 섬 가장자리를 따라 스플라인을 그린다.
 *
 * 경계가 이 스플라인 하나에만 존재한다 — 보이지 않는 벽도, 가장자리 나무도 같은 곡선에서
 * 나오므로 곡선을 고치면 둘이 같이 따라온다. 지형 모양이 상자가 아니어도 상관없다.
 *
 * 봇은 경계를 따로 읽지 않는다. 벽이 네비게이션에 참여해서 벽 바깥의 네비메시가 끊기고,
 * 봇 배회가 쓰는 GetRandomReachablePointInRadius는 도달 가능한 점만 돌려주기 때문이다.
 * 그래서 경계를 늘리면 봇의 행동 반경도 자동으로 따라온다 — 규칙이 두 벌 생기지 않는다.
 *
 * 벽은 스플라인을 토막 내 세운 박스들이다(ABeanWall이 벽을 만드는 방식과 같다).
 * 곡선 자체를 콜리전으로 쓰는 방법은 없으므로, SegmentLength를 줄이면 곡률을 더 촘촘히 따라간다.
 *
 * 배치 방법: 레벨에 하나 놓고, 스플라인 점을 지형 위에 찍어 닫힌 고리를 만든다.
 * 점을 옮긴 뒤에는 네비메시를 다시 빌드해야 봇에게도 반영된다.
 */
UCLASS()
class CHAPSALKING_API ACSKArenaBounds : public AActor
{
	GENERATED_BODY()

public:
	ACSKArenaBounds();

protected:
	// 에디터에서 스플라인을 만지는 즉시 벽과 나무가 따라오게 한다
	virtual void OnConstruction(const FTransform& Transform) override;

	// 벽과 나무는 저장되지 않는 런타임 컴포넌트다. 레벨에서 불러온 액터는 OnConstruction이
	// 돌지 않을 수 있으므로 여기서 한 번 더 세운다 — 다시 세우기 전에 기존 것을 지우므로 중복되지 않는다
	virtual void BeginPlay() override;

	// 섬 가장자리를 따라 그린다. Closed Loop를 켜야 고리가 닫힌다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Arena")
	TObjectPtr<USplineComponent> BoundarySpline;

	// 벽 한 토막의 길이. 짧을수록 곡선을 촘촘히 따라가고 컴포넌트 수가 는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Arena|Wall", meta = (ClampMin = "50.0", Units = "cm"))
	float SegmentLength = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Arena|Wall", meta = (ClampMin = "10.0", Units = "cm"))
	float WallThickness = 100.0f;

	// 널뛰기로 크게 뜨므로 넉넉히 잡는다. 낮으면 넘어가 버린다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Arena|Wall", meta = (ClampMin = "100.0", Units = "cm"))
	float WallHeight = 2000.0f;

	// 스플라인보다 지형이 내려앉은 구간에서 벽 밑이 뜨지 않게 아래로 더 묻는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Arena|Wall", meta = (ClampMin = "0.0", Units = "cm"))
	float WallSink = 500.0f;

	// 가장자리에 심을 나무. 비워두면 벽만 세운다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Arena|Props")
	TArray<TObjectPtr<UStaticMesh>> PropMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Arena|Props", meta = (ClampMin = "50.0", Units = "cm"))
	float PropSpacing = 900.0f;

	// 벽에서 바깥으로 밀어내는 거리. 나무가 안쪽에 심기면 부호를 뒤집으면 된다
	// (스플라인을 어느 방향으로 그렸느냐에 따라 오른쪽이 안이 될 수도, 밖이 될 수도 있다)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Arena|Props", meta = (Units = "cm"))
	float PropLateralOffset = 200.0f;

	// 나무가 한 줄로 서면 인공적이다 — 안팎으로 흩뿌리는 폭
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Arena|Props", meta = (ClampMin = "0.0", Units = "cm"))
	float PropLateralJitter = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Arena|Props", meta = (ClampMin = "0.01"))
	FVector2D PropScaleRange = FVector2D(0.85f, 1.35f);

	// 같은 씨앗이면 같은 숲이 나온다. 배치가 마음에 안 들면 숫자만 굴린다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Arena|Props")
	int32 PropRandomSeed = 1234;

	// 이 거리 밖의 나무는 그리지 않는다. Min에서 디더로 사라지기 시작해 Max에서 완전히 빠진다
	// (폴리지 타입의 CullDistance와 같은 방식). 섬 건너편이 비어 보이면 늘린다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Arena|Props", meta = (Units = "cm"))
	FInt32Interval PropCullDistance = FInt32Interval(18000, 20000);

	// 잎 흔들림(WPO)은 이 거리 안에서만 계산한다. 멀리서는 흔들려도 안 보이고 정점 셰이더 비용만 든다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Arena|Props", meta = (ClampMin = "0", Units = "cm"))
	int32 PropSwayDistance = 5000;

private:
	void RebuildBoundary();

	void BuildWalls(float SplineLength);
	void BuildProps(float SplineLength);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBoxComponent>> WallSegments;

	// 메시 종류마다 하나씩. 인스턴싱이라 나무가 수백 그루여도 드로우콜은 종류 수만큼이다
	UPROPERTY(Transient)
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> PropClusters;
};
