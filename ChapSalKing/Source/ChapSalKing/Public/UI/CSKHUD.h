#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CSKHUD.generated.h"

class UCSKRootHUDWidget;

/**
 * 인게임 HUD의 부트스트랩. 루트 위젯을 1회 생성해 뷰포트에 올리는 것 외에는 아무것도 하지 않는다.
 * 그리기·레이아웃·데이터 바인딩은 전부 UMG 쪽에 있다.
 *
 * 루트 위젯 클래스는 UCSKProjectSettingsData::UISettings::RootHUDWidgetClass에서 온다.
 */
UCLASS()
class CHAPSALKING_API ACSKHUD : public AHUD
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "CSK|UI")
	UCSKRootHUDWidget* GetRootHUDWidget() const
	{
		return RootHUDWidget;
	}

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UCSKRootHUDWidget> RootHUDWidget;
};
