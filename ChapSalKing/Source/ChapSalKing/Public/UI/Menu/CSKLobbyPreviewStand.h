#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSKLobbyPreviewStand.generated.h"

class ACSKPlayerCharacter;
class UCSKPlayerSoftBodyComponent;
class UDoughSoftBodyData;
class USceneComponent;
class UDoughData;
class USkeletalMeshComponent;

/**
 * 로비 슬롯에 설 반죽 하나. 레벨에 배치하지 않는다 —
 * ACSKLobbyPreviewStage가 칸 수만큼 스폰한다.
 *
 * 렌더타겟이 아니라 레벨에 실제로 서서 메인 카메라에 찍힌다. 자리와 크기는
 * Stage가 UI 슬롯의 화면 좌표에서 역산해 매 프레임 넣어주므로 여기서는 정하지 않는다.
 *
 * 외형은 인게임 폰 CDO에서 그대로 복사한다. 프리뷰 전용 메시를 따로 지정하지 않으므로
 * 캐릭터를 바꿔도 프리뷰가 뒤처지지 않는다.
 *
 * 대기 모션은 인게임과 같은 구조(소프트바디 컴포넌트 + ABP)로 돌지만 설정값은 별개다 —
 * LobbySettings.PreviewSoftBodyData를 쓰므로 로비 흔들림을 아무리 손봐도 매치에 영향이 없다.
 * (ABP는 소유 액터에서 이 컴포넌트를 찾는다 — 스탠드는 폰이 아니다)
 *
 * 이름·준비 표시는 UI(UCSKLobbySlotWidget)가 그린다 — 여기엔 반죽 외형만 있다.
 */
UCLASS()
class CHAPSALKING_API ACSKLobbyPreviewStand : public AActor
{
	GENERATED_BODY()

public:
	ACSKLobbyPreviewStand();

	// 인게임 캐릭터의 메시 컴포넌트를 본떠 외형을 맞춘다
	void InitFromPawnClass(TSubclassOf<ACSKPlayerCharacter> PawnClass);

	// 메시가 액터 정면을 보므로 그대로 두면 카메라에 뒷모습이 잡힌다.
	// 액터가 아니라 메시만 돌린다 — 액터의 방향은 Stage가 카메라에 맞춰 잡는다.
	// InitFromPawnClass 다음에 부를 것
	void SetPreviewYaw(float Yaw);

	// 프리뷰 전용 대기 흔들림 설정. 컴포넌트가 BeginPlay에서 이 값을 요구하므로
	// FinishSpawning 전에 불러야 한다
	void SetSoftBodyData(UDoughSoftBodyData* SoftBodyData);

	// nullptr이면 빈 자리 — 스탠드를 숨긴다
	void SetDough(UDoughData* DoughData);

	// 로비 화면이 아닐 때 숨긴다. 빈 자리는 화면과 무관하게 계속 숨어 있다
	void SetStageVisible(bool bVisible);

	// 스케일 1일 때 반죽의 세로 길이(cm). Stage가 칸 높이에 맞출 배율을 여기서 낸다
	float GetBaseHeight() const
	{
		return BaseHeight;
	}

	// 스케일 1일 때 반죽의 가로 지름(cm). 칸이 가로로 빡빡할 때 이쪽이 배율을 정한다.
	// 반죽 뒤에 세울 카드의 깊이도 이 값의 절반에서 나온다
	float GetBaseWidth() const
	{
		return BaseWidth;
	}

	// 스케일 1일 때 액터 원점에서 반죽 시각 중심까지의 높이(cm).
	// 메시가 캡슐 기준으로 아래에 달려 있어 이만큼 올려야 칸 중심에 온다
	float GetBaseCenterZ() const
	{
		return BaseCenterZ;
	}

protected:
	// 캐릭터의 캡슐 자리. 메시가 이 아래 붙어야 캐릭터와 같은 오프셋을 쓸 수 있다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Lobby")
	TObjectPtr<USceneComponent> StandRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Lobby")
	TObjectPtr<USkeletalMeshComponent> DoughMesh;

	// 대기 흔들림의 출처. ABP가 여기 AnimState를 읽는다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Lobby")
	TObjectPtr<UCSKPlayerSoftBodyComponent> SoftBodyComponent;

private:
	void UpdateVisibility();

	// 폰에서 복사한 원래 메시 회전. 프리뷰 각도는 여기에 더해서 준다
	FRotator MeshBaseRotation = FRotator::ZeroRotator;

	float BaseHeight = 0.0f;
	float BaseWidth = 0.0f;
	float BaseCenterZ = 0.0f;

	bool bHasDough = false;
	bool bStageVisible = false;
};
