#include "UI/CSKEventFeedWidget.h"

#include "Framework/CSKGameState.h"
#include "UI/CSKEventFeedRowWidget.h"

#include "Components/PanelWidget.h"
#include "GameFramework/PlayerState.h"

UCSKEventFeedWidget::UCSKEventFeedWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BroadcastEventTypes = {
		ECSKMatchEventType::KingAppeared,
		ECSKMatchEventType::KingEnded,
		ECSKMatchEventType::DoughEaten,
		ECSKMatchEventType::DoughEscaped,
		ECSKMatchEventType::SwallowedBySnake
	};

	LocalOnlyEventTypes = {
		ECSKMatchEventType::ItemAcquired
	};
}

void UCSKEventFeedWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ACSKGameState* CSKGameState = GetCSKGameState())
	{
		CSKGameState->OnMatchEvent.AddDynamic(this, &UCSKEventFeedWidget::HandleMatchEvent);
	}
}

void UCSKEventFeedWidget::NativeDestruct()
{
	if (ACSKGameState* CSKGameState = GetCSKGameState())
	{
		CSKGameState->OnMatchEvent.RemoveDynamic(this, &UCSKEventFeedWidget::HandleMatchEvent);
	}

	Super::NativeDestruct();
}

void UCSKEventFeedWidget::HandleMatchEvent(const FCSKMatchEventInfo& EventInfo)
{
	if (!ShouldDisplay(EventInfo))
	{
		return;
	}

	if (!RowWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] EventFeed: RowWidgetClass 미설정"));
		return;
	}

	// 새 줄을 올리기 전에 자리를 비운다. 한 프레임이라도 MaxRows를 넘기면 피드가 튄다
	while (RowContainer->GetChildrenCount() >= MaxRows)
	{
		RowContainer->RemoveChildAt(bNewestOnTop ? RowContainer->GetChildrenCount() - 1 : 0);
	}

	UCSKEventFeedRowWidget* Row =
		CreateWidget<UCSKEventFeedRowWidget>(GetOwningPlayer(), RowWidgetClass);
	if (!Row)
	{
		return;
	}

	RowContainer->AddChild(Row);
	if (bNewestOnTop)
	{
		RowContainer->ShiftChild(0, Row);
	}

	Row->SetEvent(EventInfo, RowLifetime);
}

bool UCSKEventFeedWidget::ShouldDisplay(const FCSKMatchEventInfo& EventInfo) const
{
	if (BroadcastEventTypes.Contains(EventInfo.EventType))
	{
		return true;
	}

	if (!LocalOnlyEventTypes.Contains(EventInfo.EventType))
	{
		return false;
	}

	const APlayerState* LocalPlayerState = GetOwningPlayerState();
	return LocalPlayerState != nullptr
		&& (EventInfo.EventInstigator.Get() == LocalPlayerState || EventInfo.EventTarget.Get() == LocalPlayerState);
}
