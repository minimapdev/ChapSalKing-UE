#pragma once

#include "CoreMinimal.h"
#include "UI/Menu/CSKMenuWidget.h"
#include "CSKMenuRootWidget.generated.h"

class UImage;
class UWidgetSwitcher;

/**
 * 프론트엔드의 유일한 루트. ACSKMenuHUD가 1회 생성해 뷰포트에 올린다.
 *
 * 화면 전환은 ShowScreen 하나로만 한다. WBP_CSKMenuRoot의 ScreenSwitcher에
 * ECSKMenuScreen 순서 그대로 화면을 넣어야 한다 — 중간을 비우면 뒤 화면의 인덱스가
 * 앞당겨져 엉뚱한 화면이 뜬다.
 * 분위기가 크게 바뀌는 전환(인트로 ↔ 타이틀)은 TransitionToScreen으로 막을 덮었다 걷으며 바꾼다.
 *
 * 첫 화면은 기록(UCSKProfileSubsystem)이 정한다 — 인트로를 아직 안 봤으면 Intro, 아니면 Title.
 * 시작할 때는 막이 덮인 채로 첫 화면을 띄우고 걷어 낸다.
 * 키 입력을 직접 받는 화면(IsFocusable, 예: 인트로)이 활성이면 루트가 받은 포커스를 그 화면에 넘긴다.
 */
UCLASS()
class CHAPSALKING_API UCSKMenuRootWidget : public UCSKMenuWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CSK|Menu")
	void ShowScreen(ECSKMenuScreen Screen);

	// 막으로 덮고 → ShowScreen → 걷어 낸다. 막이 덮인 동안의 입력은 막이 받아 버린다.
	// 덮이는 중에 다시 부르면 마지막 요청한 화면으로 간다
	UFUNCTION(BlueprintCallable, Category = "CSK|Menu")
	void TransitionToScreen(ECSKMenuScreen Screen);

	UFUNCTION(BlueprintPure, Category = "CSK|Menu")
	ECSKMenuScreen GetCurrentScreen() const
	{
		return CurrentScreen;
	}

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ACSKMenuHUD가 UI 입력 모드로 루트에 포커스를 준다 — 활성 화면이 키를 직접 받는다면 그쪽으로 넘긴다
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

private:
	enum class EVeilPhase : uint8
	{
		Idle,
		Covering,
		Revealing,
	};

	UCSKMenuWidget* GetScreenWidget(ECSKMenuScreen Screen) const;
	void BeginReveal();

	// 막이 다 덮이는(걷히는) 데 걸리는 시간
	UPROPERTY(EditAnywhere, Category = "CSK|Menu|Transition", meta = (ClampMin = "0.05", Units = "s"))
	float VeilSeconds = 0.45f;

	// 슬롯 순서 = ECSKMenuScreen 순서
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> ScreenSwitcher;

	// 화면 전체를 덮는 막. 모든 화면보다 위에 있어야 한다
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> TransitionVeil;

	ECSKMenuScreen CurrentScreen = ECSKMenuScreen::Title;

	EVeilPhase VeilPhase = EVeilPhase::Idle;
	float VeilElapsed = 0.0f;
	ECSKMenuScreen PendingScreen = ECSKMenuScreen::Title;
};
