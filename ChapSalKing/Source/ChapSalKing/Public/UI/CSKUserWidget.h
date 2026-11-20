#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSKUserWidget.generated.h"

class ACSKGameState;
class ACSKPlayerState;
class ACSKPlayerCharacter;
class UCSKUIStyleData;

/**
 * 모든 CSK 위젯의 공통 부모.
 *
 * - GameState/PlayerState/폰 접근을 한 곳에 모은다
 * - 폰에 붙는 구독은 BindToPawn/UnbindFromPawn 한 쌍에서만 한다.
 *   폰 교체(리스폰)는 루트 HUD가 RebindPawn을 호출해 처리하므로 위젯은 재생성되지 않는다
 * - GameState처럼 매치 내내 살아있는 대상 구독은 NativeConstruct/NativeDestruct 한 쌍에서 한다
 */
UCLASS(Abstract)
class CHAPSALKING_API UCSKUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "CSK|UI")
	ACSKGameState* GetCSKGameState() const;

	UFUNCTION(BlueprintPure, Category = "CSK|UI")
	ACSKPlayerState* GetOwningCSKPlayerState() const;

	UFUNCTION(BlueprintPure, Category = "CSK|UI")
	ACSKPlayerCharacter* GetBoundCharacter() const;

	UFUNCTION(BlueprintPure, Category = "CSK|UI")
	UCSKUIStyleData* GetUIStyle() const;

	// 루트 HUD 전용 진입점. 이전 폰을 해제하고 새 폰에 바인딩한다. 같은 폰이면 아무것도 하지 않는다
	void RebindPawn(APawn* NewPawn);

protected:
	virtual void NativeDestruct() override;

	// 폰 종속 구독은 이 한 쌍에서만. Super 호출 불필요
	virtual void BindToPawn(ACSKPlayerCharacter* Character) {}
	virtual void UnbindFromPawn(ACSKPlayerCharacter* Character) {}

private:
	TWeakObjectPtr<ACSKPlayerCharacter> BoundCharacter;
};
