#pragma once

#include "CoreMinimal.h"
#include "UI/Menu/CSKMenuWidget.h"
#include "CSKTitleWidget.generated.h"

class UButton;

/**
 * 첫 화면. 싱글/멀티/종료 세 갈래와 인트로 다시 보기를 담당한다.
 * 레이아웃은 WBP_CSKTitle — 여기 있는 이름 그대로 버튼을 만들어야 붙는다.
 */
UCLASS()
class CHAPSALKING_API UCSKTitleWidget : public UCSKMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleSingleClicked();

	UFUNCTION()
	void HandleMultiClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UFUNCTION()
	void HandleReplayIntroClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SingleButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MultiButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReplayIntroButton;
};
