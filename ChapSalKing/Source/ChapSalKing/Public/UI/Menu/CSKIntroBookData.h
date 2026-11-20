#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "CSKIntroBookData.generated.h"

class UTexture2D;
class USoundBase;

/**
 * 펼침 본문이 놓일 자리.
 * 그림을 뽑을 때 비워 둔 면과 맞춘다 — Docs/ChapSalKing_IntroStorybook.md 1절의 "본문 자리".
 */
UENUM(BlueprintType)
enum class ECSKIntroTextAnchor : uint8
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
};

USTRUCT(BlueprintType)
struct FCSKIntroSpread
{
	GENERATED_BODY()

	// 양면 일러스트 한 장 (가로 3:2)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro")
	TObjectPtr<UTexture2D> Art;

	// 리치 텍스트. <sfx>…</> = 의성어, <name>…</> = 이름 강조. 스타일은 WBP의 TextStyleSet이 정한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro", meta = (MultiLine = true))
	FText Body;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro")
	ECSKIntroTextAnchor TextAnchor = ECSKIntroTextAnchor::TopLeft;
};

/**
 * 동화책 인트로 한 권. UCSKIntroBookWidget이 표지 → 펼침 순서대로 넘긴다.
 * 그림·본문의 원본은 ArtSource/Intro/Storybook/Pages 와 Docs/ChapSalKing_IntroStorybook.md 2절.
 */
UCLASS(BlueprintType)
class CHAPSALKING_API UCSKIntroBookData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	// 표지 (세로 2:3). 로고·부제는 WBP가 얹는다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro")
	TObjectPtr<UTexture2D> CoverArt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro")
	TArray<FCSKIntroSpread> Spreads;

	// 표지가 열리기 전 머무는 시간. 화면이 막 뒤에서 떠오르는 시간(UCSKMenuRootWidget::VeilSeconds)을 포함한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float CoverHoldSeconds = 2.4f;

	// 책장 한 장이 넘어가는 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Timing", meta = (ClampMin = "0.1", Units = "s"))
	float TurnSeconds = 1.25f;

	// 본문이 서서히 나타나는 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Timing", meta = (ClampMin = "0.1", Units = "s"))
	float TextFadeSeconds = 0.6f;

	// 본문이 다 뜬 뒤 다음 장까지 = ReadBaseSeconds + 글자 수 × ReadSecondsPerCharacter
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float ReadBaseSeconds = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float ReadSecondsPerCharacter = 0.06f;

	// 이만큼 누르고 있으면 건너뛴다. 이보다 짧게 눌렀다 떼면 다음 장
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Timing", meta = (ClampMin = "0.1", Units = "s"))
	float SkipHoldSeconds = 1.0f;

	// 본문 카드가 나타나며 떠오르는 거리 (펼침 그림 픽셀 기준)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Motion", meta = (ClampMin = "0.0"))
	float TextRiseDistance = 16.0f;

	// 펼침을 읽는 동안 그림이 천천히 다가오는 정도 — 1.03이면 읽는 시간 동안 3% 커진다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Motion", meta = (ClampMin = "1.0"))
	float ArtDriftScale = 1.03f;

	// 끝나거나 건너뛸 때 책이 다가오며 막 뒤로 사라지는 크기와 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Motion", meta = (ClampMin = "1.0"))
	float EndZoomScale = 1.06f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Motion", meta = (ClampMin = "0.1", Units = "s"))
	float EndZoomSeconds = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Sound")
	TObjectPtr<USoundBase> TurnSound;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
