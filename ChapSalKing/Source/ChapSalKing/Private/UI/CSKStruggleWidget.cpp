#include "UI/CSKStruggleWidget.h"

#include "Player/CSKPlayerCharacter.h"

#include "Components/ProgressBar.h"

void UCSKStruggleWidget::BindToPawn(ACSKPlayerCharacter* Character)
{
	UCSKEatableComponent* Eatable = Character->GetEatableComponent();
	if (!Eatable)
	{
		ApplyEatenState(ECSKEatenState::None);
		return;
	}

	Eatable->OnEatenStateChanged.AddDynamic(this, &UCSKStruggleWidget::HandleEatenStateChanged);
	Eatable->OnStruggleChanged.AddDynamic(this, &UCSKStruggleWidget::HandleStruggleChanged);

	StruggleBar->SetPercent(Eatable->GetStruggleRatio());
	ApplyEatenState(Eatable->GetEatenState());
}

void UCSKStruggleWidget::UnbindFromPawn(ACSKPlayerCharacter* Character)
{
	if (UCSKEatableComponent* Eatable = Character->GetEatableComponent())
	{
		Eatable->OnEatenStateChanged.RemoveDynamic(this, &UCSKStruggleWidget::HandleEatenStateChanged);
		Eatable->OnStruggleChanged.RemoveDynamic(this, &UCSKStruggleWidget::HandleStruggleChanged);
	}

	ApplyEatenState(ECSKEatenState::None);
}

void UCSKStruggleWidget::HandleEatenStateChanged(ECSKEatenState NewState)
{
	// 붙은 상태에서 사출 회복으로 넘어가는 것은 탈출뿐 (세션 종료 해제는 None으로 바로 간다)
	if (bShown && NewState == ECSKEatenState::EjectedRecovering)
	{
		BP_OnEscaped();
	}

	ApplyEatenState(NewState);
}

void UCSKStruggleWidget::HandleStruggleChanged(float Ratio)
{
	StruggleBar->SetPercent(Ratio);
}

void UCSKStruggleWidget::ApplyEatenState(ECSKEatenState NewState)
{
	const bool bShouldShow = NewState == ECSKEatenState::StuckToKing;

	// 가시성은 매번 적용 — 첫 바인딩 시 Designer 기본값(Visible)을 덮어써야 한다
	SetVisibility(bShouldShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	if (bShouldShow == bShown)
	{
		return;
	}

	bShown = bShouldShow;
	if (bShown)
	{
		BP_OnStruggleShown();
	}
	else
	{
		BP_OnStruggleHidden();
	}
}
