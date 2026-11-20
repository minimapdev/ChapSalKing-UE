#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CSKMenuHUD.generated.h"

class UCSKMenuRootWidget;

/**
 * 프론트엔드의 부트스트랩. 인게임 ACSKHUD와 같은 역할을 LV_Title에서 한다.
 * 루트 위젯을 1회 생성해 올리고 입력을 UI 전용으로 돌리는 것 외에는 아무것도 하지 않는다.
 *
 * 루트 위젯 클래스는 UCSKProjectSettingsData::UISettings::MenuRootWidgetClass에서 온다.
 */
UCLASS()
class CHAPSALKING_API ACSKMenuHUD : public AHUD
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "CSK|Menu")
	UCSKMenuRootWidget* GetMenuRootWidget() const
	{
		return MenuRootWidget;
	}

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UCSKMenuRootWidget> MenuRootWidget;
};
