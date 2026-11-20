#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TimerHandle.h"
#include "Cauldron.generated.h"

class UBoxComponent;
class UNiagaraComponent;
class ACSKPlayerCharacter;
class UCauldronData;

UENUM()
enum class ECauldronPhase : uint8
{
	Open,		// 김 오름. 탈 수 있음
	Sinking,	// 반죽이 들어가는 중 (출발 솥). 사용 중
	Closed,		// 김 없음. 탈 수 없음. 도착지 후보
	Popping		// 반죽이 나오는 중 (도착 솥). 사용 중
};

// 가마솥 텔레포트. 열린 솥에 들어가면 다른 솥에서 튀어나온다. 사양: Docs/ChapSalKing_Cauldron.md
// 열림/닫힘은 김(VFX)으로만 보인다. 뚜껑은 항상 솥 위에 있고, 반죽이 드나들 때만 제자리에서 솟았다 쿵 떨어진다.
// 짝은 없고 도착지는 무작위.
UCLASS()
class CHAPSALKING_API ACauldron : public AActor
{
	GENERATED_BODY()

public:
	ACauldron();

	bool IsOpen() const
	{
		return Phase == ECauldronPhase::Open;
	}

	// 탈 수 있는 조건 전부. 봇이 도주 목표를 고를 때 쓴다. 실제 진입은 EnterVolume 오버랩으로만
	bool CanEnter(const ACSKPlayerCharacter* Character) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Cauldron")
	void BP_OnEntered(ACSKPlayerCharacter* Character);

	// 도착 솥, 예고 시작 (덜컹·김 샘)
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Cauldron")
	void BP_OnExitWarning();

	// 뚜껑이 솟기 시작 (들어갈 때·나올 때 공용)
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Cauldron")
	void BP_OnLidLifted();

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Cauldron")
	void BP_OnCharacterEjected(ACSKPlayerCharacter* Character);

	// 뚜껑이 솥 위에 쿵
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Cauldron")
	void BP_OnLidLanded();

	// 다시 열림 (김 재개)
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Cauldron")
	void BP_OnReopened();

private:
	// 뚜껑 팝업: 제자리에서 꼭대기까지 솟았다 떨어진다. 값은 닫힘 위치 기준 Z 오프셋
	struct FLidHop
	{
		float FromZ = 0.0f;
		float ApexZ = 0.0f;
		float UpDuration = 0.0f;
		float DownDuration = 0.0f;
		float Elapsed = 0.0f;
		bool bActive = false;

		void Start(float InFromZ, float InApexZ, float InUp, float InDown);
		// 진행시키고 현재 Z 오프셋을 반환. 끝나면 bActive=false
		float Advance(float DeltaSeconds);
	};

	enum class EPopStep : uint8
	{
		Warning,	// TransitDelay 대기
		LidUp,		// 뚜껑 솟는 중. LidUpEjectDelay 뒤 반죽 사출
		LidDown,	// 사출 끝. 뚜껑 착지 대기
	};

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	// 입구 감지. 널뛰기 BoardVolume과 같은 설정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Cauldron", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> EnterVolume;

	// 뚜껑 축. BP에서 뚜껑 메시를 붙이면 C++가 위아래로 움직인다. 기본 위치 = 솥 위
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Cauldron", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> LidRoot;

	// 들어가는 목표점 = 사출 시작점
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Cauldron", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> PotInterior;

	// 튀어나온 반죽의 착지점. 레벨에서 배치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Cauldron", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> ExitLandingPoint;

	// 열림 상태 김. 이것이 "탈 수 있음" 표시의 전부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Cauldron", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> SteamEffect;

	UPROPERTY(EditAnywhere, Category = "CSK|Cauldron")
	TObjectPtr<UCauldronData> CauldronData;

	ECauldronPhase Phase = ECauldronPhase::Open;
	float PhaseElapsed = 0.0f;
	EPopStep PopStep = EPopStep::Warning;

	// Sinking 중 (출발 솥)
	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Cauldron")
	TObjectPtr<ACSKPlayerCharacter> CapturedCharacter;
	bool bCapturedHidden = false;
	ECollisionResponse CapturedPawnResponse = ECR_Block;

	// Popping 중 (도착 솥)
	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Cauldron")
	TObjectPtr<ACSKPlayerCharacter> ArrivingCharacter;
	ECollisionResponse ArrivingPawnResponse = ECR_Block;

	// 사출 후 비행 중. 착지에서 잠금 해제
	TWeakObjectPtr<ACSKPlayerCharacter> FlyingCharacter;

	// 뚜껑
	FVector LidBaseLocation = FVector::ZeroVector;	// LidRoot 기본 상대 위치 (솥 위)
	float LidHeight = 0.0f;							// 현재 Z 오프셋
	FLidHop LidHop;

	FTimerHandle ReopenTimerHandle;

	UFUNCTION()
	void OnEnterOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void TryEnter(ACSKPlayerCharacter* Character);
	void TickSinking(float DeltaSeconds);
	// 진입 호 비행 끝 (ArcFlight 콜백)
	void HandleSunk(const FVector& ArrivalLocation);
	// 들어간 뒤: 숨기고 도착 솥을 고른다. 후보가 없으면 다음 틱에 다시
	void HandOff();
	void ReceiveArrival(ACSKPlayerCharacter* Character, ECollisionResponse SavedPawnResponse);
	void TickPopping(float DeltaSeconds);
	void Eject();
	void HandleLanded(const FVector& ArrivalLocation);
	void EnterClosed();
	void Reopen();
	void AbortSinking();
	// 이동 중 매치 종료: 숨김 상태로 남기지 않는다
	void RevealArrivingImmediately();

	ACauldron* PickDestination() const;

	void HideCharacter(ACSKPlayerCharacter* Character, ECollisionResponse& OutSavedResponse) const;
	void ShowCharacter(ACSKPlayerCharacter* Character, ECollisionResponse SavedResponse) const;

	void StartLidHop();
	// 뚜껑 팝업을 진행한다. 착지한 틱에 true
	bool TickLid(float DeltaSeconds);
	bool IsMatchFinished() const;
};
