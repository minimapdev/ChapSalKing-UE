#pragma once

#include "CoreMinimal.h"
#include "UI/CSKUserWidget.h"
#include "CSKCrosshairWidget.generated.h"

class UProgressBar;
class UIngredientData;
class ACSKPlayerCharacter;

/**
 * 조준점. 재료 슈팅을 조준하는 동안에만 보인다.
 *
 * 표시 조건은 UCSKPlayerShootComponent의 조준 상태 하나뿐이다.
 * 조준 자체가 "반죽 상태 + 재료 보유"를 요구하고, 시퀀스 잠금은 조준을 해제하므로
 * 킹·재료·시퀀스 조건이 전부 여기에 이미 포함돼 있다.
 *
 * 요구 위젯: CooldownRing (ProgressBar)
 */
UCLASS(Abstract)
class CHAPSALKING_API UCSKCrosshairWidget : public UCSKUserWidget
{
	GENERATED_BODY()

protected:
	virtual void BindToPawn(ACSKPlayerCharacter* Character) override;
	virtual void UnbindFromPawn(ACSKPlayerCharacter* Character) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 발사 가능 여부(쿨다운 완료). 조준 중에만 의미가 있다
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Crosshair")
	void BP_OnCanShootChanged(bool bCanShoot);

	// 보유 재료가 바뀔 때. 계열 컬러는 UIStyleData::GetAffinityColor로
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Crosshair")
	void BP_OnIngredientChanged(UIngredientData* Ingredient);

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Crosshair", meta = (BindWidget))
	TObjectPtr<UProgressBar> CooldownRing;

private:
	UFUNCTION()
	void HandleShootStateChanged(bool bAiming, float CooldownRatio);

	UFUNCTION()
	void HandleIngredientStateChanged(UIngredientData* Ingredient, float GaugeRatio);

	void SetAimingVisible(bool bAiming);

	// 쿨다운 완료 상태가 바뀌었을 때만 BP에 알린다
	void RefreshCanShoot();

	bool bCanShootDisplayed = false;
};
