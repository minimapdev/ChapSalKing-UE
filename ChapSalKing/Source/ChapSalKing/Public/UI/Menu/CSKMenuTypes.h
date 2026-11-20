#pragma once

#include "CoreMinimal.h"
#include "CSKMenuTypes.generated.h"

/**
 * 프론트엔드 화면 목록.
 *
 * UCSKMenuRootWidget의 WidgetSwitcher 슬롯 순서가 이 열거형 순서와 1:1로 대응한다.
 * WBP에서 화면을 추가할 때는 여기 순서대로 넣을 것.
 *
 * 캐릭터 선택은 독립 화면이 아니다 — 싱글은 봇으로 채워진 로비, 멀티는 사람이 들어오는
 * 로비이고, 색 고르기는 양쪽 로비 안의 패널 하나다.
 *
 * Intro는 첫 실행에 Title보다 먼저 뜨지만 맨 뒤에 둔다 — 앞에 끼우면 기존 슬롯 인덱스가 전부 밀린다.
 */
UENUM(BlueprintType)
enum class ECSKMenuScreen : uint8
{
	Title,
	RoomList,
	Lobby,
	Intro,
};

/**
 * 로비 슬롯 한 칸이 화면에서 차지하는 자리.
 *
 * 로비 위젯(UI)이 재서 ACSKLobbyPreviewStage(월드)에 넘긴다 — 3D 반죽의 자리를
 * UI 격자가 정하게 하는 통로다. 좌표는 뷰포트 픽셀이라 DPI 스케일이 이미 반영돼 있고,
 * 그대로 역투영에 넣을 수 있다.
 */
struct FCSKLobbySlotViewportRect
{
	// 칸의 중심
	FVector2D Center = FVector2D::ZeroVector;

	// 칸의 세로·가로 길이. 반죽은 둘 중 빡빡한 쪽에 맞춰 크기가 잡히고,
	// 칸 바탕 카드는 이 사각형을 그대로 덮는다
	float Height = 0.0f;
	float Width = 0.0f;
};
