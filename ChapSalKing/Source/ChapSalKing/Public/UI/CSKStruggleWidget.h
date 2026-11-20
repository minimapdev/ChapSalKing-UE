#pragma once

#include "CoreMinimal.h"
#include "UI/CSKUserWidget.h"
#include "Player/Components/CSKEatableComponent.h"
#include "CSKStruggleWidget.generated.h"

class UProgressBar;

/**
 * 내가 킹 몸에 붙어 있을 때만 보이는 몸부림 게이지.
 * 좌우 입력 전환으로 차오르고, 다 차면 탈출한다.
 *
 * 요구 위젯: StruggleBar (ProgressBar)
 */
UCLASS(Abstract)
class CHAPSALKING_API UCSKStruggleWidget : public UCSKUserWidget
{
	GENERATED_BODY()

protected:
	virtual void BindToPawn(ACSKPlayerCharacter* Character) override;
	virtual void UnbindFromPawn(ACSKPlayerCharacter* Character) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Struggle")
	void BP_OnStruggleShown();

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Struggle")
	void BP_OnStruggleHidden();

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Struggle")
	void BP_OnEscaped();

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Struggle", meta = (BindWidget))
	TObjectPtr<UProgressBar> StruggleBar;

private:
	UFUNCTION()
	void HandleEatenStateChanged(ECSKEatenState NewState);

	UFUNCTION()
	void HandleStruggleChanged(float Ratio);

	void ApplyEatenState(ECSKEatenState NewState);

	bool bShown = false;
};
