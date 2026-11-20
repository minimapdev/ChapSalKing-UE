#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Seesaw.generated.h"

class UBoxComponent;
class ACSKPlayerCharacter;
class AMortarZone;
class USeesawData;

enum class ESeesawPhase : uint8
{
	Idle,
	Boarding,
	Springing,
	Resetting
};

// 절구공이 운반자의 변신 진입점.
// 대기 상태에서 RidePoint(탑승 발판)가 내려와 있고 StompPoint가 올라가 있다.
// 절구공이를 든 플레이어가 발판에 올라서면 → StompPoint가 쿵 내려찍히고
// RidePoint가 그 힘으로 솟구치며 → 달 절구까지 포물선 발사.
// 발사 직후 널판은 곧바로 대기 상태로 되돌아온다.
UCLASS()
class CHAPSALKING_API ASeesaw : public AActor
{
	GENERATED_BODY()

public:
	ASeesaw();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Seesaw")
	void BP_OnSeesawBoarded(ACSKPlayerCharacter* Character);

	// 널판이 튕기기 시작하는 순간 — 사운드/이펙트는 BP에서 (널판 회전은 C++가 구동)
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Seesaw")
	void BP_OnSeesawSpring(ACSKPlayerCharacter* Character);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Seesaw")
	void BP_OnSeesawLaunched(ACSKPlayerCharacter* Character);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	// 널판 회전축. BP의 널판 메시를 여기에 붙이면 함께 기울어진다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Seesaw", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> PlankRoot;

	// 탑승 발판. 대기 상태에서 내려와 있고, 튕길 때 솟구치며 탑승자를 쏘아올린다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Seesaw", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> RidePoint;

	// 반대편 발판. 대기 상태에서 올라가 있고, 튕길 때 쿵 내려찍힌다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Seesaw", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> StompPoint;

	// 탑승 감지. 널판과 함께 움직이도록 PlankRoot에 붙어 있다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Seesaw", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> BoardVolume;

	// 목표 절구. 미지정 시 월드의 첫 절구
	UPROPERTY(EditInstanceOnly, Category = "CSK|Seesaw")
	TSoftObjectPtr<AMortarZone> TargetMortar;

	UPROPERTY(Transient)
	TObjectPtr<USeesawData> SeesawData;

	// 널판 위에 올라타 있는 캐릭터 (Boarding~Springing)
	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Seesaw")
	TObjectPtr<ACSKPlayerCharacter> CapturedCharacter;

	UPROPERTY(Transient)
	TObjectPtr<AMortarZone> CapturedMortar;

	// 발사되어 비행 중인 캐릭터. 널판은 이미 원위치로 돌아오지만 이 동안 재탑승은 막힌다
	UPROPERTY(Transient)
	TObjectPtr<ACSKPlayerCharacter> FlyingCharacter;

	UPROPERTY(Transient)
	TObjectPtr<AMortarZone> FlyingMortar;

	ESeesawPhase Phase = ESeesawPhase::Idle;
	float PhaseElapsed = 0.0f;
	float ResetStartTilt = 0.0f;
	FVector BoardStartLocation = FVector::ZeroVector;
	FRotator PlankBaseRotation = FRotator::ZeroRotator;

	UFUNCTION()
	void OnBoardOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	bool CanStartLaunch(const ACSKPlayerCharacter* Character) const;
	void TryStartLaunch(ACSKPlayerCharacter* Character);
	void TickBoarding(float DeltaSeconds);
	void TickSpringing(float DeltaSeconds);
	void TickResetting(float DeltaSeconds);
	void LaunchToMortar();
	void HandleArrival(const FVector& ArrivalLocation);
	void AbortLaunch();
	void BeginReset(float FromTilt);
	void ReturnToIdle();

	// Pitch = +TiltAngle이면 RidePoint가 내려온 대기 상태, -TiltAngle이면 튕겨올린 상태
	void SetPlankTilt(float PitchDegrees);
	// 탑승자를 RidePoint 발판 위에 올려둔다
	void SnapCharacterToRidePoint() const;
	AMortarZone* ResolveMortar() const;
};
