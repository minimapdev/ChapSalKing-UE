#include "UI/CSKScreenEffectsWidget.h"

#include "Player/CSKPlayerCharacter.h"

#include "Components/Image.h"

void UCSKScreenEffectsWidget::BindToPawn(ACSKPlayerCharacter* Character)
{
	UCSKPlayerStatusComponent* StatusComponent = Character->GetStatusComponent();
	if (!StatusComponent)
	{
		return;
	}

	StatusComponent->OnStatusChanged.AddDynamic(
		this, &UCSKScreenEffectsWidget::HandleStatusChanged);

	// 폰 교체 직후 이미 걸려 있던 상태를 반영
	ApplyStatus(ECSKStatusType::VisionBlocked, StatusComponent->HasStatus(ECSKStatusType::VisionBlocked));
}

void UCSKScreenEffectsWidget::UnbindFromPawn(ACSKPlayerCharacter* Character)
{
	if (UCSKPlayerStatusComponent* StatusComponent = Character->GetStatusComponent())
	{
		StatusComponent->OnStatusChanged.RemoveDynamic(
			this, &UCSKScreenEffectsWidget::HandleStatusChanged);
	}

	// 폰이 바뀌면 남아 있던 오버레이를 끈다
	ApplyStatus(ECSKStatusType::VisionBlocked, false);
}

void UCSKScreenEffectsWidget::HandleStatusChanged(
	ECSKStatusType Type, bool bActive, float RemainingDuration)
{
	ApplyStatus(Type, bActive);
	BP_OnStatusChanged(Type, bActive, RemainingDuration);
}

void UCSKScreenEffectsWidget::ApplyStatus(ECSKStatusType Type, bool bActive)
{
	// 혼란은 화면 오버레이 없이 BP_OnStatusChanged로만 전달한다
	if (Type != ECSKStatusType::VisionBlocked)
	{
		return;
	}

	VisionBlockOverlay->SetVisibility(
		bActive ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
