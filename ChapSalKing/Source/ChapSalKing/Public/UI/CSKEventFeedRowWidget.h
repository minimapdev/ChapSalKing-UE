#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "Framework/CSKMatchEventTypes.h"
#include "UI/CSKUserWidget.h"
#include "CSKEventFeedRowWidget.generated.h"

class UTextBlock;

/**
 * 전황 피드 한 줄. 피드가 생성해 컨테이너에 꽂고, 수명이 지나면 스스로 사라진다.
 *
 * 문구와 색은 UCSKUIStyleData가 준다 — 위젯도 BP도 문구를 조립하지 않는다.
 * 그래서 "누가 먹혔다"의 말투를 바꾸려고 WBP를 열 일이 없다.
 *
 * 요구 위젯: EventText (TextBlock)
 */
UCLASS(Abstract)
class CHAPSALKING_API UCSKEventFeedRowWidget : public UCSKUserWidget
{
	GENERATED_BODY()

public:
	// 문구·색을 채우고 수명 타이머를 건다
	void SetEvent(const FCSKMatchEventInfo& EventInfo, float Lifetime);

protected:
	virtual void NativeDestruct() override;

	// 등장~퇴장 애니메이션은 여기서. 재생 길이를 피드의 RowLifetime과 맞춘다.
	// bLocalPlayerInvolved면 내가 낀 사건이다 — 강조는 BP가 정한다
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Feed")
	void BP_OnEventSet(const FCSKMatchEventInfo& EventInfo, bool bLocalPlayerInvolved);

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Feed", meta = (BindWidget))
	TObjectPtr<UTextBlock> EventText;

private:
	void HandleLifetimeExpired();

	FTimerHandle LifetimeTimerHandle;
};
