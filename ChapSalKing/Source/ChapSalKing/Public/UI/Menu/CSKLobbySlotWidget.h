#pragma once

#include "CoreMinimal.h"
#include "UI/Menu/CSKMenuWidget.h"
#include "CSKLobbySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UWidget;
class UDoughData;

/**
 * 로비 참가자 슬롯 한 칸. 레이아웃은 WBP_CSKLobbySlot.
 *
 * 칸의 카드 바탕과 반죽은 뒤에 선 3D(ACSKLobbyPreviewStage)가 그린다. UMG는 언제나
 * 그 위에 오므로 이 위젯이 배경을 칠하면 반죽을 통째로 가린다 — 여기서 그리는 것은
 * 반죽 위에 얹혀야 하는 것들뿐이다: 이름표, 내 자리 표시.
 *
 * 3D 카드가 맞출 사각형은 칸 전체가 아니라 CardArea다. 칸은 격자가 정하는 대로
 * 늘었다 줄었다 하지만 CardArea는 ScaleBox가 카드 텍스처 비율을 지킨 채 칸 안에
 * 넣어 주므로 카드가 늘어나는 일이 없다. 카드 자리는 WBP가 소유한다.
 */
UCLASS()
class CHAPSALKING_API UCSKLobbySlotWidget : public UCSKMenuWidget
{
	GENERATED_BODY()

public:
	// DoughData가 nullptr이면 빈 자리
	void SetSlotData(UDoughData* DoughData, bool bInIsLocalPlayer);

	// 3D 카드와 반죽이 맞출 사각형. 칸 전체가 아니라 카드가 실제로 놓이는 자리다
	const FGeometry& GetCardGeometry() const;

private:
	// 카드 텍스처와 같은 비율로 잡아 둔 상자
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> CardArea;

	// 빈 자리에서는 이름표째로 사라진다 — 글자만 비우면 바탕이 덩그러니 남는다
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> NameFrame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SlotNameText;

	// 내 자리에만 보이는 테두리. 글로우든 무엇이든 WBP가 한 덩어리로 묶어 두면
	// 코드는 켜고 끄기만 한다
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> SelectionFrame;

	// 내 자리 카드 위에 얹히는 왕관
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CrownImage;
};
