#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LuckyPouch.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UIngredientData;
class APestleItem;
class ACSKPlayerCharacter;

/**
 * 하늘에서 떨어지는 복주머니. 맵의 모든 보급이 이것 하나로 온다 — 지상 재료 스폰은 없다.
 *
 * 안에 무엇이 들었는지는 닿아 봐야 안다. 절구공이도 이 안에 숨어 오므로, 웨이브가 떨어지는
 * 순간 플레이어는 "넷 중 하나가 절구공이"라는 것만 알고 어느 것인지는 모른다.
 *
 * 새 조작을 요구하지 않는다: 열기 입력도 들고 다니기도 없고, 닿으면 끝난다. 경합은 규칙이
 * 아니라 물리가 만든다 — 그림자 마커를 보고 여럿이 몰려드는 것 자체가 밀치기 싸움이고,
 * 착지 후 물렁하게 튀고 굴러가므로 예고 지점과 최종 위치가 다르다.
 *
 * 절구공이는 매치 내내 단일 액터로 유지되므로 주머니가 새로 만들지 않는다. 게임모드가
 * 숨겨 둔 절구공이를 넘겨주고, 주머니는 열릴 때 그 자리에 드러내기만 한다.
 */
UCLASS()
class CHAPSALKING_API ALuckyPouch : public AActor
{
	GENERATED_BODY()

public:
	ALuckyPouch();

	// 그림자 마커 → 낙하 → 착지 순서를 시작한다. 게임모드만 호출한다.
	// InPestle이 있으면 절구공이 주머니이고, 없으면 LootType에 따라 재료 또는 폭탄이다
	void StartDrop(
		const FVector& InLandingLocation,
		UIngredientData* InIngredient,
		bool bInIsBomb,
		APestleItem* InPestle);

	bool IsPestlePouch() const
	{
		return StashedPestle != nullptr;
	}

	// 아직 그림자 단계이거나 떨어지는 중이면 참. 봇은 이 동안 목표로 삼지 않는다
	bool IsFalling() const
	{
		return Phase != EPouchPhase::Landed;
	}

	const FVector& GetLandingLocation() const
	{
		return LandingLocation;
	}

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// 그림자 마커가 뜬 순간. 이펙트를 코드가 하나 띄우지만, 그 이상은 BP가 소유한다
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|LuckyPouch")
	void BP_OnShadowShown(const FVector& InLandingLocation, float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|LuckyPouch")
	void BP_OnLanded();

	// bWasPestle / bWasBomb 둘 다 거짓이면 재료였다
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|LuckyPouch")
	void BP_OnOpened(bool bWasPestle, bool bWasBomb);

private:
	enum class EPouchPhase : uint8
	{
		Shadow,		// 그림자만 떠 있다. 주머니는 아직 숨어 있다
		Falling,	// 보간으로 내려온다
		Landed		// 물리에 맡겼다. 주울 수 있다
	};

	UFUNCTION()
	void OnPickupOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void BeginFall();
	void Land();
	void Open(ACSKPlayerCharacter* Opener);
	void Explode();
	void HandleAutoOpen();
	void HandleLifetimeExpired();

	// 루트가 픽업 판정이자 물리 바디다. 주머니는 물렁한 덩어리 하나이므로 둘을 나눌 이유가 없다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|LuckyPouch", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> PouchBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|LuckyPouch", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PouchMesh;

	// 내용물. 셋 다 비어 있으면 잘못 초기화된 것이다
	UPROPERTY(Transient)
	TObjectPtr<UIngredientData> Ingredient;

	UPROPERTY(Transient)
	TObjectPtr<APestleItem> StashedPestle;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> ShadowEffectComponent;

	bool bIsBomb = false;
	bool bOpened = false;

	EPouchPhase Phase = EPouchPhase::Shadow;

	// 그림자 마커가 찍히는 지면 위의 점
	FVector LandingLocation = FVector::ZeroVector;

	// 실제로 주머니가 놓이는 곳 — LandingLocation에서 구 반경만큼 띄운 자리.
	// 중심을 지면에 두면 물리를 켜는 순간 절반이 땅에 박혀 뚫고 내려간다
	FVector RestLocation = FVector::ZeroVector;
	float DropHeight = 0.0f;
	float FallDuration = 0.0f;
	float FallElapsed = 0.0f;

	FTimerHandle PhaseTimerHandle;
};
