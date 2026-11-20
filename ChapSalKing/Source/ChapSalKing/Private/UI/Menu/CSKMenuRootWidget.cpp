#include "UI/Menu/CSKMenuRootWidget.h"

#include "Framework/CSKProfileSubsystem.h"

#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"

void UCSKMenuRootWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// ACSKMenuHUD가 이 위젯을 UI 전용 입력의 포커스 대상으로 넘긴다.
	// UUserWidget은 기본이 포커스 불가라 그대로 두면 키보드·게임패드 조작이 죽는다
	SetIsFocusable(true);

	const bool bIntroSeen = GetGameInstance()->GetSubsystem<UCSKProfileSubsystem>()->HasSeenIntro();
	ShowScreen(bIntroSeen ? ECSKMenuScreen::Title : ECSKMenuScreen::Intro);

	// 첫 화면도 막 뒤에서 떠오른다 — 로딩 직후 화면이 툭 튀어나오지 않게
	BeginReveal();
}

void UCSKMenuRootWidget::TransitionToScreen(ECSKMenuScreen Screen)
{
	PendingScreen = Screen;
	if (VeilPhase == EVeilPhase::Covering)
	{
		return;
	}

	// 걷히는 중이었다면 지금의 짙기에서 다시 덮기 시작한다 — 막이 깜빡이지 않게
	VeilElapsed = VeilPhase == EVeilPhase::Revealing ? VeilSeconds - VeilElapsed : 0.0f;
	VeilPhase = EVeilPhase::Covering;
	TransitionVeil->SetVisibility(ESlateVisibility::Visible);
}

void UCSKMenuRootWidget::BeginReveal()
{
	VeilPhase = EVeilPhase::Revealing;
	VeilElapsed = 0.0f;
	TransitionVeil->SetRenderOpacity(1.0f);
	TransitionVeil->SetVisibility(ESlateVisibility::Visible);
}

void UCSKMenuRootWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (VeilPhase == EVeilPhase::Idle)
	{
		return;
	}

	VeilElapsed += InDeltaTime;
	const float Alpha = FMath::SmoothStep(0.0f, 1.0f, FMath::Min(VeilElapsed / VeilSeconds, 1.0f));

	if (VeilPhase == EVeilPhase::Covering)
	{
		TransitionVeil->SetRenderOpacity(Alpha);
		if (Alpha >= 1.0f)
		{
			ShowScreen(PendingScreen);
			BeginReveal();
		}
		return;
	}

	TransitionVeil->SetRenderOpacity(1.0f - Alpha);
	if (Alpha >= 1.0f)
	{
		VeilPhase = EVeilPhase::Idle;
		TransitionVeil->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCSKMenuRootWidget::ShowScreen(ECSKMenuScreen Screen)
{
	const int32 SlotIndex = static_cast<int32>(Screen);
	if (SlotIndex >= ScreenSwitcher->GetNumWidgets())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CSK] MenuRoot: %s 화면이 ScreenSwitcher에 없습니다 (슬롯 %d개)"),
			*UEnum::GetValueAsString(Screen), ScreenSwitcher->GetNumWidgets());
		return;
	}

	// 나가는 화면부터 정리한다 — 로비가 세워둔 3D 반죽이 다음 화면에 남지 않게.
	// 첫 호출(NativeConstruct)에서는 나갈 화면이 없다
	UCSKMenuWidget* PreviousWidget = CurrentScreen != Screen ? GetScreenWidget(CurrentScreen) : nullptr;
	if (PreviousWidget)
	{
		PreviousWidget->NativeOnScreenDeactivated();
	}

	CurrentScreen = Screen;
	ScreenSwitcher->SetActiveWidgetIndex(SlotIndex);

	UCSKMenuWidget* ScreenWidget = GetScreenWidget(Screen);
	if (ScreenWidget)
	{
		ScreenWidget->NativeOnScreenActivated();
	}

	// 키를 직접 받는 화면은 포커스를 가져가고, 그런 화면을 떠날 때는 루트가 되찾는다 —
	// 접힌 화면에 포커스가 남으면 다음 화면의 키보드·게임패드 조작이 죽는다
	if (ScreenWidget && ScreenWidget->IsFocusable())
	{
		ScreenWidget->SetKeyboardFocus();
	}
	else if (PreviousWidget && PreviousWidget->IsFocusable())
	{
		SetKeyboardFocus();
	}
}

FReply UCSKMenuRootWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	UCSKMenuWidget* ScreenWidget = GetScreenWidget(CurrentScreen);
	if (ScreenWidget && ScreenWidget->IsFocusable())
	{
		return FReply::Handled().SetUserFocus(ScreenWidget->TakeWidget(), InFocusEvent.GetCause());
	}

	return Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
}

UCSKMenuWidget* UCSKMenuRootWidget::GetScreenWidget(ECSKMenuScreen Screen) const
{
	return Cast<UCSKMenuWidget>(ScreenSwitcher->GetWidgetAtIndex(static_cast<int32>(Screen)));
}
