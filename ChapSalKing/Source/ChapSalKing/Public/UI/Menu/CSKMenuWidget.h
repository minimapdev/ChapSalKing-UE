#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Menu/CSKMenuTypes.h"
#include "CSKMenuWidget.generated.h"

class UCSKGameInstance;
class UCSKMenuRootWidget;
class UCSKUIStyleData;

/**
 * 모든 프론트엔드 위젯의 공통 부모.
 *
 * 인게임 UCSKUserWidget과 계열이 다르다. 프론트엔드에는 폰도 GameState도 없고,
 * 유일한 데이터 소유자는 레벨 전환을 넘어 살아남는 UCSKGameInstance다.
 *
 * 위젯 트리는 WBP가 소유한다. C++은 meta=(BindWidget)으로 필요한 칸만 집어오고
 * 배치·색·폰트에는 관여하지 않는다 — 이름이 WBP와의 계약이므로 함부로 바꾸지 말 것.
 */
UCLASS()
class CHAPSALKING_API UCSKMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 선택 결과(반죽·봇 인원)를 담아 매치 레벨로 넘기는 유일한 통로
	UFUNCTION(BlueprintPure, Category = "CSK|Menu")
	UCSKGameInstance* GetCSKGameInstance() const;

	// 화면 전환은 루트가 독점한다. 중첩 깊이와 무관하게 같은 루트를 돌려준다
	UFUNCTION(BlueprintPure, Category = "CSK|Menu")
	UCSKMenuRootWidget* GetMenuRoot() const;

	// 화면 전환 알림. UCSKMenuRootWidget::ShowScreen이 보낸다.
	// WidgetSwitcher는 비활성 자식의 Visibility를 건드리지 않으므로
	// 위젯이 스스로 "지금 내가 보이는 화면인지"를 알 방법이 이것뿐이다.
	// 월드에 물건을 세우는 화면(로비)은 여기서 켜고 꺼야 한다
	virtual void NativeOnScreenActivated() {}
	virtual void NativeOnScreenDeactivated() {}

protected:
	UCSKUIStyleData* GetUIStyle() const;
};
