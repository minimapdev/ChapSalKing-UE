#pragma once

#include "CoreMinimal.h"
#include "UI/CSKUserWidget.h"
#include "Framework/CSKGameState.h"
#include "CSKResultWidget.generated.h"

class UPanelWidget;
class UButton;
class UCSKScoreRowWidget;

/**
 * 매치 결과. Finished일 때만 보인다. 순위 행은 스코어보드와 같은 행 위젯을 쓴다.
 *
 * WBP 요구 위젯: RowContainer (PanelWidget), RestartButton / QuitButton (Button)
 * WBP 필수 설정: RowWidgetClass
 */
UCLASS(Abstract)
class CHAPSALKING_API UCSKResultWidget : public UCSKUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Result")
	TSubclassOf<UCSKScoreRowWidget> RowWidgetClass;

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Result")
	void BP_OnResultShown();

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Result", meta = (BindWidget))
	TObjectPtr<UPanelWidget> RowContainer;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Result", meta = (BindWidget))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Result", meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

private:
	UFUNCTION()
	void HandleMatchPhaseChanged(ECSKMatchPhase NewPhase);

	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleQuitClicked();

	void ApplyPhase(ECSKMatchPhase Phase);
	void ShowResults();
};
