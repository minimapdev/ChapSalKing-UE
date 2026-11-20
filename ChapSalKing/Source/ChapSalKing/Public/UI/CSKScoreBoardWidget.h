#pragma once

#include "CoreMinimal.h"
#include "UI/CSKUserWidget.h"
#include "CSKScoreBoardWidget.generated.h"

class UPanelWidget;
class UCSKScoreRowWidget;
class ACSKPlayerCharacter;

/**
 * 몸집(G) 순위표. OnScoreboardChanged / 킹 변경 시에만 갱신한다.
 * 행 위젯은 재사용한다 — 매번 새로 만들지 않는다.
 *
 * WBP 요구 위젯: RowContainer (VerticalBox 등 PanelWidget)
 * WBP 필수 설정: RowWidgetClass
 */
UCLASS(Abstract)
class CHAPSALKING_API UCSKScoreBoardWidget : public UCSKUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Score")
	TSubclassOf<UCSKScoreRowWidget> RowWidgetClass;

	// 0이면 전원 표시. 압축 표시에서 상위 N명만 보여줄 때 쓴다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Score", meta = (ClampMin = "0"))
	int32 MaxRows = 0;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Score", meta = (BindWidget))
	TObjectPtr<UPanelWidget> RowContainer;

private:
	UFUNCTION()
	void HandleScoreboardChanged();

	UFUNCTION()
	void HandleChapSalKingChanged(ACSKPlayerCharacter* NewKing);

	void Rebuild();
};
