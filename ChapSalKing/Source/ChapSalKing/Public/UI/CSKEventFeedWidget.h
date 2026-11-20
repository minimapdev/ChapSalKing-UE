#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKMatchEventTypes.h"
#include "UI/CSKUserWidget.h"
#include "CSKEventFeedWidget.generated.h"

class UPanelWidget;
class UCSKEventFeedRowWidget;

/**
 * 전황 피드. 매치 중 일어난 사건을 한 줄씩 쌓는다.
 *
 * 무엇을 띄울지는 두 집합이 정한다.
 * - BroadcastEventTypes: 전원에게 보인다. 판을 뒤집는 사건(킹·먹힘·뱀)이 여기다
 * - LocalOnlyEventTypes: 내가 당사자일 때만 보인다. 남의 것까지 띄우면 피드가 잠기는
 *   사건(재료 획득)이 여기다. 8인전에서 8초마다 도는 보급을 전부 줄로 쓰면 피드가 보급 로그가 된다
 *
 * 낙하 계열(PouchDropped / PestleRespawned)은 기본값에서 빠져 있다. 상시로 일어나는 일은
 * 알림이 아니라 배경이고, 줄로 알리면 정작 중요한 줄이 밀려난다.
 *
 * WBP 요구 위젯: RowContainer (VerticalBox 등 PanelWidget)
 * WBP 필수 설정: RowWidgetClass
 */
UCLASS(Abstract)
class CHAPSALKING_API UCSKEventFeedWidget : public UCSKUserWidget
{
	GENERATED_BODY()

public:
	UCSKEventFeedWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Feed")
	TSubclassOf<UCSKEventFeedRowWidget> RowWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Feed")
	TSet<ECSKMatchEventType> BroadcastEventTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Feed")
	TSet<ECSKMatchEventType> LocalOnlyEventTypes;

	// 넘치면 가장 오래된 줄부터 걷어낸다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Feed", meta = (ClampMin = "1"))
	int32 MaxRows = 5;

	// 한 줄이 화면에 머무는 시간. 행 WBP의 등장~퇴장 애니메이션 길이와 맞춘다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Feed", meta = (ClampMin = "0.5", Units = "s"))
	float RowLifetime = 5.0f;

	// 참이면 새 줄이 맨 위에 꽂힌다. 거짓이면 아래로 쌓인다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Feed")
	bool bNewestOnTop = true;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Feed", meta = (BindWidget))
	TObjectPtr<UPanelWidget> RowContainer;

private:
	UFUNCTION()
	void HandleMatchEvent(const FCSKMatchEventInfo& EventInfo);

	bool ShouldDisplay(const FCSKMatchEventInfo& EventInfo) const;
};
