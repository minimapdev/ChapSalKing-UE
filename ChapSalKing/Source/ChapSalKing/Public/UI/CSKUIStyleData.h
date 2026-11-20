#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "Combat/CSKAffinityTypes.h"
#include "Framework/CSKMatchEventTypes.h"
#include "CSKUIStyleData.generated.h"

class UTexture2D;

/**
 * UI 공용 스타일. 색·아이콘의 유일한 정의처 — 위젯에 색을 하드코딩하지 않는다.
 * 인스턴스는 UCSKProjectSettingsData::UISettings에서 지정한다.
 */
UCLASS(BlueprintType)
class CHAPSALKING_API UCSKUIStyleData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	// 계열 컬러 — 재료 탄약/크로스헤어/상태 아이콘이 공유한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style|Color")
	TMap<ECSKAffinity, FLinearColor> AffinityColors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style|Color")
	FLinearColor NeutralColor = FLinearColor::White;

	// 몸집 증가 / 감소 피드백
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style|Color")
	FLinearColor PositiveColor = FLinearColor(0.25f, 0.85f, 0.35f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style|Color")
	FLinearColor NegativeColor = FLinearColor(0.9f, 0.25f, 0.25f);

	// 매치 잔여 30초 이하 / 킹 세션 5초 이하 경고
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style|Color")
	FLinearColor UrgentColor = FLinearColor(1.0f, 0.45f, 0.1f);

	// 스코어보드에서 내 행을 표시하는 색
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style|Color")
	FLinearColor LocalPlayerColor = FLinearColor(1.0f, 0.9f, 0.35f);

	// 스코어보드 1위 / 킹 표시
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style|Icon")
	TObjectPtr<UTexture2D> KingCrownIcon;

	// 전황 피드 한 줄의 문구. {0}=주인공 {1}=대상 {2}=부가 이름(재료명 등).
	// 예) KingAppeared → "{0}이(가) 찹쌀킹이 되었다!", DoughEaten → "{1}이(가) {0}에게 먹혔다"
	// 여기에 없는 종류는 피드에 뜨지 않는다 — 문구가 곧 표시 여부다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style|Feed")
	TMap<ECSKMatchEventType, FText> EventFeedFormats;

	// 종류별 행 색. 매핑이 없으면 NeutralColor. 계열이 붙은 사건은 계열 색이 이긴다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style|Feed")
	TMap<ECSKMatchEventType, FLinearColor> EventFeedColors;

	// 프론트엔드(메뉴)에는 여기서 내려보내는 값이 없다 — 레이아웃·색·폰트·아트를
	// 전부 WBP가 소유하고, 코드는 위젯을 켜고 끄기만 한다.

	// 매핑이 없는 계열은 NeutralColor를 돌려준다
	UFUNCTION(BlueprintPure, Category = "Style|Color")
	FLinearColor GetAffinityColor(ECSKAffinity Affinity) const;

	// 매핑이 없는 종류는 NeutralColor를 돌려준다
	UFUNCTION(BlueprintPure, Category = "Style|Feed")
	FLinearColor GetEventFeedColor(ECSKMatchEventType EventType) const;
};
