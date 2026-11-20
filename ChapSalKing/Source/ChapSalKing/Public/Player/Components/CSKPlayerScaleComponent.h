#pragma once

#include "CoreMinimal.h"
#include "Player/CSKPlayerComponentBase.h"
#include "CSKPlayerScaleComponent.generated.h"

class UCurveFloat;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FCSKPlayerScaleChangedSignature,
	float,
	BodyScale,
	float,
	AppliedScale
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FCSKPlayerScaleTransitionFinishedSignature,
	float,
	AppliedScale
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FCSKPlayerMassChangedSignature,
	float,
	MassGrams
);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKPlayerScaleComponent : public UCSKPlayerComponentBase
{
	GENERATED_BODY()

public:
	UCSKPlayerScaleComponent();

	UFUNCTION(BlueprintCallable, Category = "CSK|Mass")
	void AddMass(float DeltaGrams);

	UFUNCTION(BlueprintCallable, Category = "CSK|Mass")
	float RemoveMass(float DeltaGrams);

	UFUNCTION(BlueprintCallable, Category = "CSK|Mass")
	void SetMass(float NewMassGrams);

	UFUNCTION(BlueprintCallable, Category = "CSK|Mass")
	void ResetMass();

	UFUNCTION(BlueprintPure, Category = "CSK|Mass")
	float GetMass() const
	{
		return MassGrams;
	}

	// 최저 몸집 위로 남은 여유분. 뜯기는 양의 기준이다.
	// 현재 몸집에 비율을 곱하면 서너 번 만에 바닥에 닿고, 바닥에 닿은 반죽은
	// 더 잃을 게 없어서 킹과 뱀을 무시하게 된다. 여유분 기준이면 바닥에 점근할 뿐 닿지 않는다
	UFUNCTION(BlueprintPure, Category = "CSK|Mass")
	float GetLossableMass() const;

	UFUNCTION(BlueprintCallable, Category = "CSK|Scale")
	void SetChapSalKingScaleBonusEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "CSK|Scale")
	void EndChapSalKingScaleBonus();

	UFUNCTION(BlueprintPure, Category = "CSK|Scale")
	bool IsChapSalKingScaleBonusEnabled() const
	{
		return bChapSalKingScaleBonusEnabled;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Scale")
	float GetBodyScale() const;

	UFUNCTION(BlueprintPure, Category = "CSK|Scale")
	float GetAppliedScale() const
	{
		return CurrentAppliedScale;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Scale")
	float GetScaleTransitionDuration() const;

	UFUNCTION(BlueprintPure, Category = "CSK|Scale")
	UCurveFloat* GetScaleTransitionCurve() const;

	UFUNCTION(BlueprintPure, Category = "CSK|Scale")
	float GetChapSalKingScaleBonus() const;

	UFUNCTION(BlueprintPure, Category = "CSK|Scale")
	float GetCameraDistanceScalePower() const;

	UPROPERTY(BlueprintAssignable, Category = "CSK|Scale")
	FCSKPlayerScaleChangedSignature OnScaleChanged;

	UPROPERTY(BlueprintAssignable, Category = "CSK|Scale")
	FCSKPlayerScaleTransitionFinishedSignature OnScaleTransitionFinished;

	UPROPERTY(BlueprintAssignable, Category = "CSK|Mass")
	FCSKPlayerMassChangedSignature OnMassChanged;

	// 현재 질량을 소유자의 PlayerState 점수로 밀어넣는다.
	// 질량이 바뀔 때 자동으로 호출되지만, 초기 질량은 possess 전에 정해지므로
	// PlayerState가 붙는 시점(NotifyControllerChanged)에 캐릭터가 한 번 더 호출한다
	void PushScoreToPlayerState() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TObjectPtr<UCurveFloat> ActiveTransitionCurve;

	float MassGrams = 0.0f;
	float CurrentAppliedScale = 1.0f;
	bool bChapSalKingScaleBonusEnabled = false;

	float StartAppliedScale = 1.0f;
	float TargetAppliedScale = 1.0f;
	float TransitionElapsedTime = 0.0f;
	float ActiveTransitionDuration = 0.0f;

	void InitMass();
	void ApplyScale();

	float CalculateAppliedScale() const;
	float ComputeBodyScaleFromMass() const;
	float ClampMass(float InMass) const;

	void StartScaleTransition();
	void ApplyCurrentScale();
};
