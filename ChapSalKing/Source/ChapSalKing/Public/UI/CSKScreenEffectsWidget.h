#pragma once

#include "CoreMinimal.h"
#include "UI/CSKUserWidget.h"
#include "Player/Components/CSKPlayerStatusComponent.h"
#include "CSKScreenEffectsWidget.generated.h"

class UImage;
class ACSKPlayerCharacter;

/**
 * 화면 전체를 덮는 상태이상 오버레이. 최하위 레이어에 둔다.
 *
 * 캐릭터 몸에 붙는 연출은 UCSKPlayerStatusVFXComponent 담당 — 여기서 중복하지 않는다.
 * 상태 컴포넌트를 직접 구독하므로 캐릭터→컨트롤러 경유 호출이 필요 없다.
 *
 * 요구 위젯: VisionBlockOverlay (Image)
 */
UCLASS(Abstract)
class CHAPSALKING_API UCSKScreenEffectsWidget : public UCSKUserWidget
{
	GENERATED_BODY()

protected:
	virtual void BindToPawn(ACSKPlayerCharacter* Character) override;
	virtual void UnbindFromPawn(ACSKPlayerCharacter* Character) override;

	// 페이드 인/아웃은 BP 애니메이션으로. RemainingDuration은 켜질 때만 유효
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|ScreenEffects")
	void BP_OnStatusChanged(ECSKStatusType Type, bool bActive, float RemainingDuration);

	UPROPERTY(BlueprintReadOnly, Category = "CSK|ScreenEffects", meta = (BindWidget))
	TObjectPtr<UImage> VisionBlockOverlay;

private:
	UFUNCTION()
	void HandleStatusChanged(ECSKStatusType Type, bool bActive, float RemainingDuration);

	void ApplyStatus(ECSKStatusType Type, bool bActive);
};
