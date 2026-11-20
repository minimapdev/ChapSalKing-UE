#pragma once

#include "CoreMinimal.h"
#include "UI/CSKUserWidget.h"
#include "CSKRootHUDWidget.generated.h"

/**
 * 인게임 HUD의 유일한 루트. ACSKHUD가 1회 생성해 뷰포트에 올린다.
 *
 * 자식 위젯은 BindWidget으로 열거하지 않는다. 위젯 트리를 순회해
 * UCSKUserWidget 자손 전부에게 폰 바인딩을 전파하므로, 에디터에서 자식을
 * 추가·제거해도 C++ 수정 없이 동작한다.
 */
UCLASS(Abstract)
class CHAPSALKING_API UCSKRootHUDWidget : public UCSKUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
};
