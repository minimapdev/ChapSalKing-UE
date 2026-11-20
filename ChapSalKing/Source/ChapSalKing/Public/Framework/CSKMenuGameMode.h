#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Dough/DoughTypes.h"
#include "CSKMenuGameMode.generated.h"

class ACSKLobbyPreviewStage;
class APlayerController;
struct FCSKLobbySlotViewportRect;

/**
 * LV_Title 전용 GameMode.
 *
 * ACSKGameMode를 상속하지 않는다 — 프론트엔드에는 매치도, 절구공이도, 재료도 없다.
 * 폰을 스폰하지 않으며, 화면은 전부 UI라 레벨에 배치할 액터가 하나도 없다.
 *
 * 로비 프리뷰는 이 클래스가 만드는 유일한 월드 객체이고, 로비 위젯이 월드에 닿는
 * 유일한 통로이기도 하다. 나중에 로비가 별도 레벨로 분리돼도 위젯은 그대로 옮겨간다.
 */
UCLASS()
class CHAPSALKING_API ACSKMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACSKMenuGameMode();

	void ApplyLobbyRoster(const TArray<EDoughCharacterType>& Roster);

	// 로비 화면에 들어오고 나갈 때의 스위치. 스탠드는 레벨에 실제로 서 있어서
	// 끄지 않으면 타이틀 화면에도 그대로 보인다
	void SetLobbyPreviewVisible(bool bVisible);

	// 슬롯 칸의 화면 자리에 3D 반죽을 맞춘다. 로비 위젯이 매 프레임 부른다
	void AlignLobbyPreview(
		APlayerController* PlayerController,
		const TArray<FCSKLobbySlotViewportRect>& SlotRects);

private:
	ACSKLobbyPreviewStage* GetOrCreatePreviewStage();

	UPROPERTY()
	TObjectPtr<ACSKLobbyPreviewStage> PreviewStage;
};
