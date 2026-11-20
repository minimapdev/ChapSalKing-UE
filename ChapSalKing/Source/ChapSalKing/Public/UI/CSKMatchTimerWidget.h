#pragma once

#include "CoreMinimal.h"
#include "UI/CSKUserWidget.h"
#include "Framework/CSKGameState.h"
#include "CSKMatchTimerWidget.generated.h"

class UTextBlock;

/**
 * 매치 남은 시간. mm:ss.
 *
 * WBP 요구 위젯: TimeText (TextBlock)
 */
UCLASS(Abstract)
class CHAPSALKING_API UCSKMatchTimerWidget : public UCSKUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 이 시간 아래로 내려가면 BP_OnUrgentChanged(true). 색·펄스는 BP가 처리한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Timer", meta = (ClampMin = "0.0", Units = "s"))
	float UrgentThreshold = 30.0f;

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Timer")
	void BP_OnUrgentChanged(bool bInUrgent);

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Timer", meta = (BindWidget))
	TObjectPtr<UTextBlock> TimeText;

private:
	UFUNCTION()
	void HandleMatchPhaseChanged(ECSKMatchPhase NewPhase);

	void ApplyPhase(ECSKMatchPhase Phase);
	void UpdateDisplay(float TimeRemaining);

	// 초 단위로만 갱신하기 위한 직전 값. -1 = 아직 그린 적 없음
	int32 DisplayedSeconds = -1;
	bool bUrgentDisplayed = false;
};
