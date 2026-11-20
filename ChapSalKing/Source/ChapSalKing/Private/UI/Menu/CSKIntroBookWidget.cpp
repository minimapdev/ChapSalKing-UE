#include "UI/Menu/CSKIntroBookWidget.h"

#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProfileSubsystem.h"
#include "Framework/CSKProjectSettingsData.h"
#include "UI/Menu/CSKIntroBookData.h"
#include "UI/Menu/CSKMenuRootWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/RetainerBox.h"
#include "Components/RichTextBlock.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace CSKIntroFlip
{
	// M_UI_PageFlip의 파라미터 — 머티리얼과의 계약
	const FName Progress = TEXT("Progress");  // 0 = 오른쪽에 평평, 1 = 왼쪽에 내려앉음
	const FName Side = TEXT("Side");          // 0 = 앞면 겹(왼쪽 절반 고정 + 오른쪽 장의 앞면), 1 = 뒷면 겹
	const FName ShowLeft = TEXT("ShowLeft");  // 앞면 겹의 왼쪽 절반을 보일지 (닫힌 책엔 왼쪽 장이 없다)
	const FName Aspect = TEXT("Aspect");      // 펼침의 가로/세로 비 — 말림이 찌그러지지 않게
}

namespace
{
	// 리치 텍스트 태그(<sfx>, </>)를 뺀 실제 글자 수 — 읽는 시간 계산용
	int32 CountReadableCharacters(const FString& RichText)
	{
		int32 Count = 0;
		bool bInTag = false;
		for (const TCHAR Ch : RichText)
		{
			if (Ch == TEXT('<'))
			{
				bInTag = true;
			}
			else if (Ch == TEXT('>'))
			{
				bInTag = false;
			}
			else if (!bInTag && !FChar::IsWhitespace(Ch))
			{
				++Count;
			}
		}
		return Count;
	}
}

void UCSKIntroBookWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 키 입력을 직접 받는 화면 — 루트가 활성화 때 포커스를 넘겨준다
	SetIsFocusable(true);

	Book = UCSKGameInstance::GetProjectSettings(this)->UISettings.IntroBookData.LoadSynchronous();
	checkf(Book, TEXT("UISettings.IntroBookData가 비어 있습니다 — 인트로 화면을 띄울 수 없습니다"));
}

void UCSKIntroBookWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// RetainerBox는 Slate 위젯이 만들어질 때 이펙트 머티리얼의 MID를 만든다 — 그래서 Construct에서 집는다
	UnderFlip = UnderRetainer->GetEffectMaterial();
	FrontFlip = PageRetainer->GetEffectMaterial();
	BackFlip = BackRetainer->GetEffectMaterial();
	checkf(UnderFlip && FrontFlip && BackFlip, TEXT("WBP_CSKIntroBook의 RetainerBox에 Effect Material(M_UI_PageFlip)이 없습니다"));

	UnderFlip->SetScalarParameterValue(CSKIntroFlip::Side, 0.0f);
	FrontFlip->SetScalarParameterValue(CSKIntroFlip::Side, 0.0f);
	BackFlip->SetScalarParameterValue(CSKIntroFlip::Side, 1.0f);

	// 세 겹 모두 펼침 그림과 같은 비율의 칸에 그려진다
	const UTexture2D* SpreadArt = Book->Spreads[0].Art;
	const float SpreadAspect = static_cast<float>(SpreadArt->GetSizeX()) / static_cast<float>(SpreadArt->GetSizeY());
	for (UMaterialInstanceDynamic* Flip : { UnderFlip.Get(), FrontFlip.Get(), BackFlip.Get() })
	{
		Flip->SetScalarParameterValue(CSKIntroFlip::Aspect, SpreadAspect);
	}

	CoverArt->SetBrushFromTexture(Book->CoverArt);
}

void UCSKIntroBookWidget::NativeOnScreenActivated()
{
	// 타이틀의 "다시 보기"로 다시 들어올 수 있다 — 매번 닫힌 책부터
	Restart();
}

void UCSKIntroBookWidget::Restart()
{
	SpreadIndex = 0;
	bPressing = false;
	PressElapsed = 0.0f;
	SkipGauge->SetVisibility(ESlateVisibility::Hidden);

	// 닫힌 책: Front엔 표지, 표지 밑(Under)과 표지 뒷면(Back)엔 첫 펼침
	TopPage->SetVisibility(ESlateVisibility::Collapsed);
	CoverPage->SetVisibility(ESlateVisibility::HitTestInvisible);
	PrepareFlip(0);
	UnderFlip->SetScalarParameterValue(CSKIntroFlip::ShowLeft, 0.0f);
	BackRetainer->SetVisibility(ESlateVisibility::Collapsed);
	SpreadBook->SetRenderScale(FVector2D::UnitVector);
	SetClosedAmount(1.0f);

	SetPhase(EPhase::Cover);
}

void UCSKIntroBookWidget::SetPhase(EPhase NewPhase)
{
	Phase = NewPhase;
	PhaseElapsed = 0.0f;
}

void UCSKIntroBookWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (Phase == EPhase::Done)
	{
		// 루트의 막이 덮이는 동안 책이 살짝 다가온다 — 화면을 떠나는 움직임
		PhaseElapsed += InDeltaTime;
		const float Alpha = FMath::Min(PhaseElapsed / Book->EndZoomSeconds, 1.0f);
		SpreadBook->SetRenderScale(FVector2D(FMath::Lerp(1.0f, Book->EndZoomScale, FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f))));
		return;
	}

	if (bPressing)
	{
		PressElapsed += InDeltaTime;
		SkipGauge->SetPercent(FMath::Min(PressElapsed / Book->SkipHoldSeconds, 1.0f));
		if (PressElapsed >= Book->SkipHoldSeconds)
		{
			Finish();
			return;
		}
	}

	PhaseElapsed += InDeltaTime;

	switch (Phase)
	{
	case EPhase::Cover:
		// 밀어 둘 거리가 책 크기에서 나온다 — 첫 배치가 끝나기 전 프레임이 있어 매 틱 다시 맞춘다
		SetClosedAmount(1.0f);
		if (PhaseElapsed >= Book->CoverHoldSeconds)
		{
			BeginOpen();
		}
		break;

	case EPhase::Opening:
	{
		const float Alpha = FMath::Min(PhaseElapsed / Book->TurnSeconds, 1.0f);
		const float Progress = FMath::SmoothStep(0.0f, 1.0f, Alpha);
		SetFlipProgress(Progress);
		SetClosedAmount(1.0f - Progress);
		if (Alpha >= 1.0f)
		{
			// 표지가 왼쪽에 내려앉았다 — 첫 펼침을 Front로 올리고 표지를 치운다
			CoverPage->SetVisibility(ESlateVisibility::Collapsed);
			TopPage->SetVisibility(ESlateVisibility::HitTestInvisible);
			FillPage(0, TopArt, TopTextCard, TopBody);
			SetFlipProgress(0.0f);
			UnderFlip->SetScalarParameterValue(CSKIntroFlip::ShowLeft, 1.0f);
			BackRetainer->SetVisibility(ESlateVisibility::Collapsed);
			EnterReading(0);
		}
		break;
	}

	case EPhase::Reading:
	{
		// 본문은 떠오르며 나타나고, 그림은 읽는 내내 아주 천천히 다가온다
		const float TextAlpha = FMath::Min(PhaseElapsed / Book->TextFadeSeconds, 1.0f);
		TopTextCard->SetRenderOpacity(TextAlpha);
		TopTextCard->SetRenderTranslation(FVector2D(0.0f, Book->TextRiseDistance * (1.0f - FMath::InterpEaseOut(0.0f, 1.0f, TextAlpha, 2.0f))));

		const float ReadTotal = Book->TextFadeSeconds + GetReadSeconds(SpreadIndex);
		TopArt->SetRenderScale(FVector2D(FMath::Lerp(1.0f, Book->ArtDriftScale, FMath::Min(PhaseElapsed / ReadTotal, 1.0f))));
		if (PhaseElapsed >= ReadTotal)
		{
			Advance();
		}
		break;
	}

	case EPhase::Turning:
	{
		const float Alpha = FMath::Min(PhaseElapsed / Book->TurnSeconds, 1.0f);
		SetFlipProgress(FMath::SmoothStep(0.0f, 1.0f, Alpha));
		if (Alpha >= 1.0f)
		{
			// 장이 왼쪽에 내려앉았다 — 다음 펼침을 Front로 올리고 넘김을 되돌린다
			FillPage(SpreadIndex + 1, TopArt, TopTextCard, TopBody);
			SetFlipProgress(0.0f);
			BackRetainer->SetVisibility(ESlateVisibility::Collapsed);
			EnterReading(SpreadIndex + 1);
		}
		break;
	}

	case EPhase::Done:
		break;
	}
}

void UCSKIntroBookWidget::Advance()
{
	switch (Phase)
	{
	case EPhase::Cover:
		BeginOpen();
		break;

	case EPhase::Reading:
		if (PhaseElapsed < Book->TextFadeSeconds)
		{
			// 본문이 덜 떴으면 넘기지 않고 먼저 전부 띄운다
			PhaseElapsed = Book->TextFadeSeconds;
		}
		else if (SpreadIndex == Book->Spreads.Num() - 1)
		{
			Finish();
		}
		else
		{
			BeginTurn();
		}
		break;

	case EPhase::Opening:
	case EPhase::Turning:
	case EPhase::Done:
		// 넘어가는 중의 입력은 버린다 — 연타로 장을 건너뛰지 않게
		break;
	}
}

void UCSKIntroBookWidget::BeginOpen()
{
	BackRetainer->SetVisibility(ESlateVisibility::HitTestInvisible);
	UGameplayStatics::PlaySound2D(this, Book->TurnSound);
	SetPhase(EPhase::Opening);
}

void UCSKIntroBookWidget::BeginTurn()
{
	PrepareFlip(SpreadIndex + 1);
	BackRetainer->SetVisibility(ESlateVisibility::HitTestInvisible);
	UGameplayStatics::PlaySound2D(this, Book->TurnSound);
	SetPhase(EPhase::Turning);
}

void UCSKIntroBookWidget::EnterReading(int32 Index)
{
	SpreadIndex = Index;
	SetPhase(EPhase::Reading);
}

void UCSKIntroBookWidget::Finish()
{
	SetPhase(EPhase::Done);
	bPressing = false;
	SkipGauge->SetVisibility(ESlateVisibility::Hidden);

	GetGameInstance()->GetSubsystem<UCSKProfileSubsystem>()->MarkIntroSeen();
	GetMenuRoot()->TransitionToScreen(ECSKMenuScreen::Title);
}

void UCSKIntroBookWidget::FillPage(int32 Index, UImage* Art, UBorder* TextCard, URichTextBlock* Body) const
{
	const FCSKIntroSpread& Spread = Book->Spreads[Index];

	Art->SetBrushFromTexture(Spread.Art);
	Art->SetRenderScale(FVector2D::UnitVector);
	Body->SetText(Spread.Body);
	TextCard->SetRenderOpacity(0.0f);
	TextCard->SetRenderTranslation(FVector2D(0.0f, Book->TextRiseDistance));

	const bool bLeft = Spread.TextAnchor == ECSKIntroTextAnchor::TopLeft || Spread.TextAnchor == ECSKIntroTextAnchor::BottomLeft;
	const bool bTop = Spread.TextAnchor == ECSKIntroTextAnchor::TopLeft || Spread.TextAnchor == ECSKIntroTextAnchor::TopRight;

	UOverlaySlot* CardSlot = CastChecked<UOverlaySlot>(TextCard->Slot);
	CardSlot->SetHorizontalAlignment(bLeft ? HAlign_Left : HAlign_Right);
	CardSlot->SetVerticalAlignment(bTop ? VAlign_Top : VAlign_Bottom);
}

void UCSKIntroBookWidget::PrepareFlip(int32 NextIndex)
{
	FillPage(NextIndex, UnderArt, UnderTextCard, UnderBody);
	FillPage(NextIndex, BackArt, BackTextCard, BackBody);
	SetFlipProgress(0.0f);
}

void UCSKIntroBookWidget::SetFlipProgress(float Progress)
{
	FrontFlip->SetScalarParameterValue(CSKIntroFlip::Progress, Progress);
	BackFlip->SetScalarParameterValue(CSKIntroFlip::Progress, Progress);
}

void UCSKIntroBookWidget::SetClosedAmount(float Closed)
{
	// 닫힌 책은 오른쪽 장(펼침 너비의 절반)만 있다 — 그 가운데가 화면 가운데 오도록 펼침 너비의 1/4만큼 민다
	const float Shift = PageRetainer->GetCachedGeometry().GetLocalSize().X * 0.25f;
	SpreadBook->SetRenderTranslation(FVector2D(-Shift * Closed, 0.0f));
	// 왼쪽 판은 넘어간 표지의 안쪽이다 — 표지가 책등을 넘어온 뒤(닫힘 0.5 → 0) 책등에서 바깥으로 펼쳐진다.
	// 판 묶음은 책 틀 전체를 덮으므로 기본 피벗(가운데)이 곧 책등이다
	const float Unfold = 1.0f - FMath::SmoothStep(0.0f, 0.5f, Closed);
	BookLeft->SetRenderScale(FVector2D(Unfold, 1.0f));
	BookLeft->SetVisibility(Unfold > 0.0f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

float UCSKIntroBookWidget::GetReadSeconds(int32 Index) const
{
	const int32 Characters = CountReadableCharacters(Book->Spreads[Index].Body.ToString());
	return Book->ReadBaseSeconds + Characters * Book->ReadSecondsPerCharacter;
}

// ---------------------------------------------------------------- 입력

void UCSKIntroBookWidget::PressBegin()
{
	if (Phase == EPhase::Done || bPressing)
	{
		return;
	}

	bPressing = true;
	PressElapsed = 0.0f;
	SkipGauge->SetPercent(0.0f);
	SkipGauge->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCSKIntroBookWidget::PressEnd()
{
	if (!bPressing)
	{
		return;
	}

	// 건너뛰기 전에 뗐다 = 짧은 누름 = 다음
	bPressing = false;
	SkipGauge->SetVisibility(ESlateVisibility::Hidden);
	Advance();
}

FReply UCSKIntroBookWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (!InKeyEvent.IsRepeat())
	{
		PressBegin();
	}
	return FReply::Handled();
}

FReply UCSKIntroBookWidget::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	PressEnd();
	return FReply::Handled();
}

FReply UCSKIntroBookWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	PressBegin();
	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply UCSKIntroBookWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	PressEnd();
	return FReply::Handled().ReleaseMouseCapture();
}
