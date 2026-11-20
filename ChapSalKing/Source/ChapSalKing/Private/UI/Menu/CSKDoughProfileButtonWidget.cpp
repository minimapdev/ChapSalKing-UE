#include "UI/Menu/CSKDoughProfileButtonWidget.h"

#include "Dough/DoughData.h"
#include "UI/Menu/CSKLobbyWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Widget.h"

void UCSKDoughProfileButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ProfileButton->OnClicked.AddDynamic(this, &UCSKDoughProfileButtonWidget::HandleClicked);
}

void UCSKDoughProfileButtonWidget::InitProfile(UDoughData* InDoughData, UCSKLobbyWidget* InOwnerLobby)
{
	DoughData = InDoughData;
	OwnerLobby = InOwnerLobby;

	// 반죽 종류가 곧 색이다 — 스와치는 그 반죽의 몸 색 그대로다
	SwatchImage->SetColorAndOpacity(DoughData->BodyColor);
}

void UCSKDoughProfileButtonWidget::SetSelected(bool bInSelected)
{
	SelectedMarker->SetVisibility(
		bInSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

EDoughCharacterType UCSKDoughProfileButtonWidget::GetCharacterType() const
{
	return DoughData->CharacterType;
}

void UCSKDoughProfileButtonWidget::HandleClicked()
{
	OwnerLobby->SelectDough(DoughData->CharacterType);
}
