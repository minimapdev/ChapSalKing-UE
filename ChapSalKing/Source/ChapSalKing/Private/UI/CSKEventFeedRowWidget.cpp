#include "UI/CSKEventFeedRowWidget.h"

#include "UI/CSKUIStyleData.h"

#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"

namespace
{
	FText MakePlayerNameText(const APlayerState* PlayerState)
	{
		return PlayerState ? FText::FromString(PlayerState->GetPlayerName()) : FText::GetEmpty();
	}
}

void UCSKEventFeedRowWidget::SetEvent(const FCSKMatchEventInfo& EventInfo, float Lifetime)
{
	// 수명부터 건다. 아래에서 문구를 못 만들고 빠져나가도 빈 줄이 화면에 남지 않는다
	GetWorld()->GetTimerManager().SetTimer(
		LifetimeTimerHandle, this, &UCSKEventFeedRowWidget::HandleLifetimeExpired, Lifetime, false);

	const UCSKUIStyleData* Style = GetUIStyle();
	if (!Style)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] EventFeedRow: UIStyleData 미설정"));
		return;
	}

	const FText* Format = Style->EventFeedFormats.Find(EventInfo.EventType);
	if (!Format)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] EventFeedRow: %s 문구가 EventFeedFormats에 없다"),
			*UEnum::GetValueAsString(EventInfo.EventType));
		return;
	}

	FFormatOrderedArguments Arguments;
	Arguments.Add(MakePlayerNameText(EventInfo.EventInstigator));
	Arguments.Add(MakePlayerNameText(EventInfo.EventTarget));
	Arguments.Add(EventInfo.SubjectName);

	EventText->SetText(FText::Format(*Format, Arguments));

	// 계열이 붙은 사건은 계열 색이 이긴다 — "무엇을 얻었나"가 글자를 읽기 전에 색으로 온다
	EventText->SetColorAndOpacity(FSlateColor(EventInfo.Affinity != ECSKAffinity::None
		? Style->GetAffinityColor(EventInfo.Affinity)
		: Style->GetEventFeedColor(EventInfo.EventType)));

	const APlayerState* LocalPlayerState = GetOwningPlayerState();
	BP_OnEventSet(EventInfo, LocalPlayerState != nullptr
		&& (EventInfo.EventInstigator.Get() == LocalPlayerState || EventInfo.EventTarget.Get() == LocalPlayerState));
}

void UCSKEventFeedRowWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(LifetimeTimerHandle);

	Super::NativeDestruct();
}

void UCSKEventFeedRowWidget::HandleLifetimeExpired()
{
	RemoveFromParent();
}
