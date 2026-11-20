#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "CauldronData.generated.h"

// 가마솥 텔레포트의 타이밍. 사양: Docs/ChapSalKing_Cauldron.md 4장
UCLASS(BlueprintType)
class CHAPSALKING_API UCauldronData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	// 밟은 뒤 반죽이 솥 테두리 위로 호를 그리며 안으로 들어가는 시간. 킹이 잡을 수 있는 창
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cauldron|Timing", meta = (ClampMin = "0.05", Units = "s"))
	float SinkDuration = 0.3f;

	// 들어갈 때 호의 높이 (출발점과 솥 안 중간 기준). 테두리를 넘을 만큼
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cauldron|Timing", meta = (ClampMin = "0.0", Units = "cm"))
	float SinkArcHeight = 120.0f;

	// 들어가기 끝 → 도착 솥의 뚜껑이 떠오르기 시작할 때까지. 도착 솥의 예고 길이
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cauldron|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float TransitDelay = 0.8f;

	// 닫힘 → 다시 열림 (김이 다시 오른다)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cauldron|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float ReopenDelay = 8.0f;

	// 뚜껑이 제자리에서 솟는 높이. 들어갈 때와 나올 때 모두
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cauldron|Lid", meta = (ClampMin = "0.0", Units = "cm"))
	float LidHopHeight = 250.0f;

	// 떠오르는 시간 (EaseOut)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cauldron|Lid", meta = (ClampMin = "0.01", Units = "s"))
	float LidHopUpDuration = 0.25f;

	// 떨어지는 시간 (EaseIn). 착지 = 쿵
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cauldron|Lid", meta = (ClampMin = "0.01", Units = "s"))
	float LidHopDownDuration = 0.3f;

	// 나올 때: 뚜껑이 떠오르기 시작한 뒤 이 시간이 지나면 반죽이 나온다 — 뚜껑 먼저, 반죽 바로 뒤
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cauldron|Lid", meta = (ClampMin = "0.0", Units = "s"))
	float LidUpEjectDelay = 0.12f;

	// 반죽 포물선 (PotInterior → ExitLandingPoint)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cauldron|Exit", meta = (ClampMin = "0.05", Units = "s"))
	float ExitFlightDuration = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cauldron|Exit", meta = (ClampMin = "0.0", Units = "cm"))
	float ExitArcHeight = 350.0f;
};
