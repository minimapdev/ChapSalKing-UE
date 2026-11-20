#include "UI/Menu/CSKLobbySlotWidget.h"

#include "Dough/DoughData.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

void UCSKLobbySlotWidget::SetSlotData(UDoughData* DoughData, bool bInIsLocalPlayer)
{
	// 빈 자리는 카드만 남는다 — 이름표 바탕까지 함께 접는다
	NameFrame->SetVisibility(
		DoughData ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	SlotNameText->SetText(DoughData ? DoughData->DisplayName : FText::GetEmpty());

	// 내 자리 표시는 테두리와 왕관이 함께 켜진다
	const ESlateVisibility LocalPlayerVisibility =
		bInIsLocalPlayer ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	SelectionFrame->SetVisibility(LocalPlayerVisibility);
	CrownImage->SetVisibility(LocalPlayerVisibility);
}

const FGeometry& UCSKLobbySlotWidget::GetCardGeometry() const
{
	return CardArea->GetCachedGeometry();
}
