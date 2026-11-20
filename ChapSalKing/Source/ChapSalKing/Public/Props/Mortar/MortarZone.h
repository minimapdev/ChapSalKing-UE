#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MortarZone.generated.h"

class UBoxComponent;
class UCameraShakeBase;
class UNiagaraComponent;
class ACSKPlayerCharacter;
class APestleItem;
class UMortarZoneData;

enum class EMortarSequencePhase : uint8
{
	Idle,
	Rising,
	Spinning,
	Striking,
	PlayingEffect,
	Returning
};

UCLASS()
class CHAPSALKING_API AMortarZone : public AActor
{
	GENERATED_BODY()

public:
	AMortarZone();

	void NotifyPestlePickedUpInside(ACSKPlayerCharacter* Character);

	// 널뛰기 비행 도착 시 진입점 — 캐릭터는 이미 시퀀스 잠금 상태
	void BeginSequenceFromArrival(ACSKPlayerCharacter* Character);

	bool IsIdle() const
	{
		return SequencePhase == EMortarSequencePhase::Idle;
	}

	// 널뛰기 비행의 목표 지점 (캐릭터 중심 기준)
	FVector GetArrivalLocation() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Mortar")
	void BP_OnMortarActivated(ACSKPlayerCharacter* Character);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Mortar")
	void BP_OnPestleImpact(ACSKPlayerCharacter* Character);

	// 변신 완료 후 복귀 비행 발사 시점
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Mortar")
	void BP_OnReturnLaunched(ACSKPlayerCharacter* Character);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Mortar")
	void BP_OnMortarSequenceFinished(ACSKPlayerCharacter* Character);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Mortar", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> EnterVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Mortar", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> PestlePlantPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Mortar", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> PestleSpinPoint;

	// 널뛰기 비행 도착 시 캐릭터가 안착하는 지점 (절구 안)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Mortar", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> ArrivalPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Mortar", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> TransformEffect;

	// 걸어 들어와 시작하는 구식 진입 허용 (달 배치 절구는 꺼둠 — 널뛰기 도착만 유효)
	UPROPERTY(EditAnywhere, Category = "CSK|Mortar")
	bool bAllowProximityStart = false;

	// 복귀 착지 지점. 미지정 시 월드의 첫 PestleSpawnPoint (맵 중앙)
	UPROPERTY(EditInstanceOnly, Category = "CSK|Mortar")
	TSoftObjectPtr<AActor> ReturnTargetOverride;

	UPROPERTY(Transient)
	TObjectPtr<UMortarZoneData> MortarData;

	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Mortar")
	TObjectPtr<ACSKPlayerCharacter> CapturedCharacter;

	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Mortar")
	TObjectPtr<APestleItem> AnimatedPestle;

	UPROPERTY(Transient)
	TObjectPtr<UCameraShakeBase> ActiveTransformBeamShake;

	EMortarSequencePhase SequencePhase = EMortarSequencePhase::Idle;
	float PhaseElapsed = 0.0f;
	FTransform PestleStartTransform = FTransform::Identity;

	UFUNCTION()
	void OnEnterMortar(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	bool CanStartSequence(const ACSKPlayerCharacter* Character) const;
	void TryStartSequence(ACSKPlayerCharacter* Character);
	void BeginRising();
	void TickRising(float DeltaSeconds);
	void TickSpinning(float DeltaSeconds);
	void TickStriking(float DeltaSeconds);
	void BeginTransformEffect();
	void TickPlayingEffect(float DeltaSeconds);
	void LaunchReturn();
	void HandleReturnLanded(const FVector& ArrivalLocation);
	bool ResolveReturnLocation(FVector& OutLocation) const;
	void StopTransformBeamShake(bool bImmediately);
	void FinishSequence();
	void AbortSequence();
	bool CanTransformToKing(const ACSKPlayerCharacter* Character) const;

	UFUNCTION()
	void HandleTransformEffectFinished(UNiagaraComponent* FinishedComponent);
};
