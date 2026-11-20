#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Props/Snake/SnakeData.h"
#include "Snake.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class USphereComponent;
class ACSKPlayerCharacter;
class ALandscapeProxy;

/**
 * 원형 아레나를 기어다니는 중립 위험 요소. 사양: Docs/ChapSalKing_Snake.md
 *
 * 머리만 실제로 움직인다. 머리의 전부는 "목표점을 향해 최대 선회율 제한을 받으며 전진"
 * 하나뿐이고, 행동 모드는 그 목표점을 어디에 찍을지만 다르게 정한다. 모드를 추가할 때
 * MoveHead를 고치게 된다면 그 모드 설계가 틀린 것이다.
 *
 * 몸통은 판단하지 않는다. 머리가 남긴 자취를 누적거리 기준으로 샘플링할 뿐이다.
 * 시간이 아니라 누적거리로 샘플링하므로 횡단(1.6배속)에서도 마디 간격이 일정하다.
 *
 * 4단계 범위 — 모드 4종 + 전환 규칙 + 예고 + 마디 콜리전 + 잠수 + 포식.
 * 흙둔덕은 아직 없다 (사양 8장 5단계).
 *
 * 멀티 seam: 리플리케이트하는 것은 머리 하나(위치·회전·모드·잠수 여부)뿐이고 몸통은 각
 * 클라가 자기 자취 버퍼로 재구성한다. 몸통 상태를 머리에서 파생되지 않는 방식으로 계산하지 말 것.
 */
UCLASS()
class CHAPSALKING_API ASnake : public AActor
{
	GENERATED_BODY()

public:
	ASnake();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Snake")
	void BP_OnModeChanged(ESnakeMode NewMode);

	// 예고 시작. TargetLocation은 실제로 갈 목표점이다 —
	// 예고에서 보여준 방향과 실제 경로가 다르면 예고가 거짓말이 된다
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Snake")
	void BP_OnTelegraphStarted(ESnakeMode NextMode, FVector TargetLocation);

	// 예고가 끝나고 실제로 움직이기 시작한다. 예고 마커를 걷는 자리다
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Snake")
	void BP_OnTelegraphFinished();

	// 땅속에서 SurfaceTelegraphDuration 뒤에 솟는다. 나올 자리를 미리 찍어주는 훅이다
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Snake")
	void BP_OnSurfaceTelegraph(FVector GroundLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Snake")
	void BP_OnSubmerged(FVector Location);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Snake")
	void BP_OnSurfaced(FVector Location);

	// 땅속에 있는 동안 주기적으로. 지면 높이의 위치를 준다 — 여기에 흙먼지를 뿌리면
	// 지하를 달리는 자국이 된다
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Snake")
	void BP_OnSubmergeTrail(FVector GroundLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Snake")
	void BP_OnDevoured(ACSKPlayerCharacter* Victim, FVector BiteLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Snake")
	void BP_OnSpitOut(ACSKPlayerCharacter* Victim, FVector SpitLocation);

private:
	// 머리가 지나간 한 점. DistanceAlong은 매치 시작부터의 누적 이동거리다
	struct FSnakeTrailSample
	{
		FVector Location = FVector::ZeroVector;
		float DistanceAlong = 0.0f;
		// 이 지점에서의 지면 대비 높이. 음수면 땅속이다.
		// 마디가 이 값을 물려받으므로 구멍이 몸을 따라 뒤로 흘러간다
		float SurfaceOffset = 0.0f;
		// 이 지점의 지면 높이. Location.Z - SurfaceOffset 으로 되돌리면 안 된다 —
		// SurfaceOffset은 구멍을 넓히려고 더 깊은 쪽을 취하는데 Location은 보간이라 둘이 어긋난다
		float GroundZ = 0.0f;
	};

	// 삼켜져 몸통을 따라 밀려가는 중인 반죽
	struct FSnakeSwallowed
	{
		TWeakObjectPtr<ACSKPlayerCharacter> Character;
		// 삼킴 연출이 끝난 시점의 누적거리. 여기서부터 자취를 거슬러 뒤로 간다
		float TravelStartDistance = 0.0f;
		float SwallowElapsed = 0.0f;
		bool bTraveling = false;
		// 몸통 구체는 WorldDynamic이라 Pawn 응답만 꺼서는 부딪힌다.
		// 삼켜진 동안은 콜리전을 통째로 끄고, 뱉을 때 되돌린다
		ECollisionEnabled::Type SavedCollision = ECollisionEnabled::QueryAndPhysics;
	};

	// 뱉어진 직후 다시 먹히지 않게 한다. 킹에게는 통하지 않는다
	struct FSnakeBiteImmunity
	{
		TWeakObjectPtr<ACSKPlayerCharacter> Character;
		float Remaining = 0.0f;
	};

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	// 씹기·예고·삼키기를 애님으로 표현하므로 스켈레탈이다.
	// 크기는 SnakeData->HeadMeshScale이 준다 — BP에서 따로 만지지 말 것
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Snake", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> HeadMesh;

	// 마디는 인스턴스가 아니라 컴포넌트다. ISM은 트랜스폼을 매 프레임 전부 갱신할 때
	// 씬 프록시를 재생성해서 TSR/TAA의 시간 누적이 끊기고 몸통이 번진다.
	// StaticMeshComponent는 틱을 돌지 않으므로 개수가 늘어도 부담이 없다.
	// 메시·머티리얼은 SnakeData가 준다 — 런타임 생성이라 BP에서 꽂을 수 없다
	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Snake")
	TArray<TObjectPtr<UStaticMeshComponent>> SegmentMeshes;

	// 머리 콜리전. 메시와 같은 구이고, Block이 아니라 Overlap이다.
	// 4단계에서 응답을 Overlap으로 바꿔 포식 판정이 된다 (사양 5.1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Snake", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> MouthVolume;

	// 마디 하나의 막는 구. 막기만 한다 — 데미지도 포식도 없다 (사양 5.1).
	// 물리 바디가 아니라 데이터다. 막는 일은 PushOutTrappedCharacters가 거리로 수평으로만 한다 —
	// 엔진의 침투 해소는 구체 중심이 반죽보다 높아서 아래로 밀어 화면이 떨린다.
	// 컴포넌트로 두면 마디 수만큼 매 프레임 이동·반경 변경·오버랩 갱신을 치르는데 그 결과를 아무도 쓰지 않는다
	struct FSnakeSegmentBody
	{
		FVector Location = FVector::ZeroVector;
		// 혹이 지나가면 커진다. 보이는 크기와 막히는 크기가 같아야 한다
		float Radius = 0.0f;
		// 잠긴 마디는 막지 않는다 — 이 구멍이 몸을 따라 뒤로 흘러간다 (사양 3.5)
		bool bBlocking = false;
	};

	// 마디마다 하나씩. UpdateSegments가 매 프레임 채운다
	TArray<FSnakeSegmentBody> SegmentBodies;

	// 땅속에 있는 마디 위의 지면에 올리는 둔덕. 잠긴 몸통을 따라 이랑이 이어진다.
	// 이랑의 양 끝이 곧 구멍의 입구와 출구라, 따로 표시할 마커가 필요 없다
	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Snake")
	TArray<TObjectPtr<UStaticMeshComponent>> MoundMeshes;

	UPROPERTY(EditAnywhere, Category = "CSK|Snake")
	TObjectPtr<USnakeData> SnakeData;

	// 목표점과 자취를 그린다. 튜닝용
	UPROPERTY(EditAnywhere, Category = "CSK|Snake")
	bool bDrawDebug = false;

	ESnakeMode CurrentMode = ESnakeMode::Orbit;
	FVector TargetPoint = FVector::ZeroVector;
	float ModeElapsed = 0.0f;
	float ModeDuration = 0.0f;
	float CurrentSpeed = 0.0f;

	// 0보다 크면 예고 중이다. 예고 동안은 모드 시간이 흐르지 않는다
	float TelegraphRemaining = 0.0f;

	// 참이면 머리와 마디가 지형 아래로 내려간다. 잠복 전 구간 + 순회·똬리 중 주기적으로
	// 잠수 진행도 0~1. 0이면 지상, 1이면 완전히 묻힘.
	// 불리언이 아니라 이 값이 시간을 두고 움직이므로 파고드는 동작이 보인다
	float SubmergeAlpha = 0.0f;

	// 지금 잠기려 하는가. SubmergeAlpha가 향하는 목표다
	bool bWantSubmerged = false;

	// 이번 잠수의 부상 예고를 이미 냈는가. 잠길 때마다 초기화된다
	bool bSurfaceTelegraphed = false;

	// 순회·똬리 중 다음 잠수까지 남은 시간. 0 이하가 되면 SubmergeRemaining을 채운다
	float SubmergeTimer = 0.0f;
	float SubmergeRemaining = 0.0f;
	float SubmergeTrailTimer = 0.0f;

	// 실제 이동 방향에서 나오는 상하 기울기. 이동에는 영향을 주지 않는 표시용이다
	float CurrentPitch = 0.0f;

	// 매치 시작부터의 누적 이동거리. 자취 샘플링의 기준축
	float HeadDistance = 0.0f;

	// 순회·똬리가 유지하는 반경과 진행 방향(+1 / -1). 모드 시작 시 정해 모드가 끝날 때까지 유지한다
	float OrbitRadius = 0.0f;
	float OrbitDirection = 1.0f;

	// 똬리: 누적 회전각과 감김 방향(+1 감김 / -1 풀림)
	float CoilTurnedDeg = 0.0f;
	float CoilInward = 1.0f;

	// 흙 높이를 묻는 대상. BeginPlay에서 한 번 찾는다
	TWeakObjectPtr<ALandscapeProxy> Landscape;

	// DistanceAlong 오름차순. 앞이 가장 오래된 샘플이다
	TArray<FSnakeTrailSample> Trail;

	TArray<FSnakeSwallowed> Swallowed;
	TArray<FSnakeBiteImmunity> BiteImmunities;

	// StartGraceDuration 판정용
	float MatchElapsed = 0.0f;

	// 마디 레이아웃. 런타임에 변하지 않으므로 BeginPlay에서 한 번 만든다.
	// 크기·간격·콜리전이 전부 여기서 나오므로 셋이 어긋날 수 없다
	TArray<float> SegmentScales;     // 메시 스케일 (꼬리로 갈수록 TailScaleRatio까지 줄어든다)
	TArray<float> SegmentDistances;  // 머리로부터의 자취 거리. 간격이 굵기에 비례하므로 누적으로 구한다
	TArray<float> SegmentRadii;      // 메시 바운드 x 스케일. 캡슐 반지름이자 잠김 판정 기준

	void BeginMode(ESnakeMode NewMode);
	ESnakeMode PickNextMode() const;
	const FSnakeModeSettings& FindModeSettings(ESnakeMode Mode) const;
	// 모드가 끝났나. 순회는 지속시간, 횡단은 목표 도달, 똬리는 회전수, 잠복은 둘 중 먼저
	bool IsModeFinished() const;
	void UpdateTarget();
	// 잠복이 겨냥할 반죽. 모드 시작 시점에 한 번만 고른다
	FVector FindNearestPlayerLocation() const;

	// 잠복 전 구간과, 순회·똬리 중 주기적으로 잠긴다. 예고 중에는 잠기지 않는다
	void UpdateSubmergeState(float DeltaSeconds, bool bTelegraphing);
	// 땅속에서 곧 솟아오르나. 솟는 조건이 모드마다 다르므로 여기서 한 번에 판정한다
	bool IsSurfacingSoon() const;
	// 땅속에 있는 동안 지면에 자국을 남긴다
	void UpdateSubmergeTrail(float DeltaSeconds);
	void MoveHead(float DeltaSeconds, float Speed, float TurnRateDeg);
	// 몸통 안에 갇힌 반죽을 수평으로 밀어낸다. 마디들이 서로 겹쳐 있어서
	// 엔진의 침투 해소에 맡기면 빠져나갈 방향을 못 찾고 낀 채로 굳는다
	// 삼키기·몸통 이동·뱉기. 사양 5장
	void UpdateBite(float DeltaSeconds);
	bool CanBite(const ACSKPlayerCharacter* Character) const;
	void BeginSwallow(ACSKPlayerCharacter* Character);
	// 뜯은 몸집을 덩어리로 쪼개 물린 자리 좌우에 뿌린다. 뱀은 아무것도 얻지 않는다
	void ScatterMass(ACSKPlayerCharacter* Character, const FVector& BiteLocation);
	void SpitOut(FSnakeSwallowed& Entry);
	// 삼켜진 반죽인가. 밀어내기에서 제외해야 한다 — 몸통 한가운데 있는 게 정상이다
	bool IsSwallowed(const ACSKPlayerCharacter* Character) const;
	// 삼킨 것이 있으면 잠수하지 않는다 — 혹이 땅속으로 들어가면 뱉을 자리가 지형 아래가 된다
	bool IsCarryingSwallowed() const
	{
		return Swallowed.Num() > 0;
	}
	// 자취 위 한 점에서의 몸통 옆 방향
	FVector TrailRightAt(float Distance) const;

	void PushOutTrappedCharacters();
	void PushTrailSample();
	void UpdateSegments();
	// 마디·둔덕용 컴포넌트를 하나 만든다. 메시와 머티리얼은 데이터가 준다
	UStaticMeshComponent* CreateSegmentMesh(UStaticMesh* Mesh, UMaterialInterface* Material);

	// 스폰 위치의 원호를 따라 뒤로 되짚어 몸통을 채운다.
	// 이게 없으면 뱀이 한 점에서 자라나는 첫 십몇 초를 매 판 보게 된다
	// 마디의 스케일·거리·반지름을 한 번에 계산한다. 메시 바운드에서 파생되므로
	// 보이는 크기와 막히는 크기가 어긋날 수 없다
	void BuildSegmentLayout();
	void PrefillTrail();
	float RequiredTrailLength() const;
	// Distance 지점의 자취 샘플. InOutIndex는 호출 간에 이어져 전체 탐색을 피한다
	FSnakeTrailSample SampleTrail(float Distance, int32& InOutIndex) const;

	// 뱀이 딛는 높이. 소품도 지면으로 친다 — 기어가는 생물이 물건을 타고 넘는 건 자연스럽다
	float GroundZ(const FVector& Location) const;
	// 흙이 있는 높이. 랜드스케이프에 직접 묻는다 —
	// 트레이스는 첫 블로킹(가마솥 뚜껑)에서 멈춰서 그 아래 흙을 못 찾는다
	float TerrainZ(const FVector& Location) const;
	// 지금의 지면 대비 높이. 양수면 지상, 음수면 땅속
	float CurrentSurfaceOffset() const;
	float SurfaceZ(const FVector& Location) const;
	FVector PointOnArena(float AngleDeg, float Radius) const;
	float CurrentAngleDeg() const;
	bool IsMatchFinished() const;
};
