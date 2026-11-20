#pragma once

#include "CoreMinimal.h"
#include "UI/Menu/CSKMenuWidget.h"
#include "CSKIntroBookWidget.generated.h"

class UBorder;
class UCSKIntroBookData;
class UImage;
class UMaterialInstanceDynamic;
class UProgressBar;
class URetainerBox;
class URichTextBlock;
class UWidget;

/**
 * 동화책 인트로 화면. 닫힌 책의 표지가 넘어가며 펼쳐지고, 펼침이 한 장씩 왼쪽으로 넘어가 쌓인다.
 *
 * 한 장 넘기기 = 오른쪽 절반 한 장이 책등을 축으로 돌아 왼쪽에 내려앉는 것.
 * 펼침 그림은 양면 한 장이라, 넘어가는 장의 앞면 = 지금 펼침의 오른쪽 절반, 뒷면 = 다음 펼침의 왼쪽 절반이다.
 * 세 겹을 RetainerBox로 그려 M_UI_PageFlip이 절반씩 잘라 보여 준다.
 *   Under(아래) = 다음 펼침   — 오른쪽 장이 들리면 그 밑의 오른쪽 절반이 드러난다
 *   Front(위)   = 지금 펼침   — 왼쪽 절반은 그대로, 오른쪽 절반이 앞면으로 넘어간다
 *   Back(맨 위) = 다음 펼침   — 장이 책등을 넘으면 왼쪽 절반이 뒷면으로 내려앉는다
 * 다 넘어가면 다음 펼침을 Front로 옮기고 Back을 접는다 — 화면은 같은 그림이라 이음매가 없다.
 *
 * 표지: Front에 표지(오른쪽 절반)를 올린 닫힌 책으로 시작한다. 책은 표지가 화면 가운데 오도록 밀려 있고,
 * 왼쪽 절반의 책 판(BookLeft)은 접혀 있다. 표지가 넘어가는 동안 책이 가운데로 돌아오고 왼쪽 판이 책등에서 펼쳐진다.
 *
 * 움직임: 본문은 떠오르며 나타나고, 그림은 읽는 동안 아주 천천히 다가오며, 끝나면 책이 다가오며 막 뒤로 사라진다.
 * 정도와 시간은 전부 UCSKIntroBookData에 있다.
 *
 * 입력: 짧게 누르면 다음(본문이 덜 떴으면 먼저 전부 띄움), SkipHoldSeconds만큼 누르고 있으면 건너뛴다.
 * 끝나거나 건너뛰면 "봤음"을 기록하고 루트의 막 전환으로 타이틀에 넘긴다.
 *
 * 레이아웃은 WBP_CSKIntroBook — 아래 BindWidget 이름이 계약이다.
 */
UCLASS()
class CHAPSALKING_API UCSKIntroBookWidget : public UCSKMenuWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnScreenActivated() override;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	enum class EPhase : uint8
	{
		Cover,    // 닫힌 책
		Opening,  // 표지가 넘어가며 책이 펼쳐지는 중
		Reading,  // 펼침 본문이 뜨고 읽는 중
		Turning,  // 다음 펼침으로 한 장 넘어가는 중
		Done,
	};

	void Restart();
	void Advance();
	void BeginOpen();
	void BeginTurn();
	void EnterReading(int32 Index);
	void Finish();

	void PressBegin();
	void PressEnd();

	// 펼침 하나를 한 겹(Front/Under/Back)에 채운다. 본문 카드는 투명하게 두고 Front에 오른 뒤에 띄운다
	void FillPage(int32 Index, UImage* Art, UBorder* TextCard, URichTextBlock* Body) const;
	// 다음 펼침을 Under·Back에 깔고 넘길 준비를 한다
	void PrepareFlip(int32 NextIndex);
	// 넘어가는 장의 각도. 0 = 오른쪽에 평평, 1 = 왼쪽에 내려앉음
	void SetFlipProgress(float Progress);
	// 0 = 펼친 책, 1 = 닫힌 책(표지가 가운데, 왼쪽 판 없음)
	void SetClosedAmount(float Closed);
	float GetReadSeconds(int32 Index) const;
	void SetPhase(EPhase NewPhase);

	// 책 전체(그림자·판·두께·장). 닫힌 동안 표지가 가운데 오도록 왼쪽으로 민다
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> SpreadBook;

	// 책의 왼쪽 절반(그림자·표지 판·책장 두께). 닫힌 동안 접혀 있다
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> BookLeft;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URetainerBox> UnderRetainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> UnderArt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> UnderTextCard;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> UnderBody;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URetainerBox> PageRetainer;

	// Front의 내용 — 펼침(TopPage) 또는 닫힌 책의 표지(CoverPage) 중 하나만 보인다
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> TopPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> TopArt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> TopTextCard;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> TopBody;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> CoverPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CoverArt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URetainerBox> BackRetainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BackArt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> BackTextCard;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> BackBody;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> SkipGauge;

	UPROPERTY(Transient)
	TObjectPtr<const UCSKIntroBookData> Book;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> UnderFlip;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FrontFlip;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BackFlip;

	EPhase Phase = EPhase::Done;
	float PhaseElapsed = 0.0f;
	int32 SpreadIndex = 0;

	bool bPressing = false;
	float PressElapsed = 0.0f;
};
