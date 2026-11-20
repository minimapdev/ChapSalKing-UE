#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSKLobbyPreviewStage.generated.h"

class ACSKLobbyPreviewStand;
class APlayerController;
class UDoughData;
class UStaticMesh;
class UStaticMeshComponent;
struct FCSKLobbySlotViewportRect;

/**
 * 로비 슬롯에 설 반죽들을 레벨에 실제로 세운다. 레벨에 배치하지 않는다 —
 * ACSKMenuGameMode가 첫 요청 때 스폰한다.
 *
 * 렌더타겟에 찍어 UI에 붙이지 않는다. 스탠드는 메인 카메라 앞에 서서 메인 렌더를
 * 그대로 타므로 해상도·안티에일리어싱·포스트·조명이 인게임과 같다. UI는 그 위에
 * 이름과 상태만 얹는다.
 *
 * 자리는 UI가 정한다 — 로비 위젯이 슬롯 칸의 화면 좌표를 넘기면 그걸 역투영해
 * 카메라 앞 PreviewDistance 지점에 스탠드를 놓고, 칸의 화면 높이에서 배율을 역산한다.
 * 격자 크기·비율·화면 해상도가 어떻게 바뀌든 반죽이 따라가므로 배치는 전적으로
 * WBP_CSKLobby가 소유한다.
 *
 * 칸 바탕 카드와 배경 판도 여기서 세운다. UMG는 언제나 3D 위에 그려지므로
 * 반죽 뒤에 무언가를 깔려면 그것도 3D여야 한다 — UI가 그릴 수 있는 것은
 * 반죽 위에 오는 것(이름·테두리·하이라이트)뿐이다.
 *
 * 스탠드도 판도 레벨에 실제로 서 있어서, 로비 화면이 아닐 때 숨기지 않으면
 * 타이틀 화면 카메라에도 그대로 잡힌다 — SetStageVisible이 그 스위치다.
 */
UCLASS()
class CHAPSALKING_API ACSKLobbyPreviewStage : public AActor
{
	GENERATED_BODY()

public:
	ACSKLobbyPreviewStage();

	// 명단 순서대로 스탠드에 배분한다. 명단보다 칸이 많으면 남는 칸은 숨는다
	void ApplyRoster(const TArray<UDoughData*>& RosterDoughData);

	// 로비 화면에 들어오고 나갈 때의 스위치
	void SetStageVisible(bool bVisible);

	// 슬롯 칸의 화면 자리에 스탠드를 맞춘다. 로비 위젯이 매 프레임 부른다
	void AlignToSlots(
		APlayerController* PlayerController,
		const TArray<FCSKLobbySlotViewportRect>& SlotRects);

protected:
	virtual void BeginPlay() override;

private:
	// 자리는 AlignToSlots가 잡는다 — 여기서는 칸 수만큼 만들기만 한다
	void BuildStands();

	// 격자 뒤 전체를 덮어 레벨을 가린다. 개수가 하나로 정해져 있어 생성자에서 만든다
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Backdrop;

	// 판의 원본 메시. 카드는 칸 수를 런타임에 알기에 여기 들고 있다가 그때 만든다
	UPROPERTY()
	TObjectPtr<UStaticMesh> PlaneMesh;

	UPROPERTY()
	TArray<TObjectPtr<ACSKLobbyPreviewStand>> Stands;

	// 칸 하나를 정확히 덮는 바탕. 빈 자리에도 깔려야 하므로 스탠드가 아니라 여기 붙는다
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> SlotCards;
};
