#pragma once

#include "CoreMinimal.h"
#include "UI/Menu/CSKMenuWidget.h"
#include "Dough/DoughTypes.h"
#include "CSKLobbyWidget.generated.h"

class UButton;
class UTextBlock;
class UUniformGridPanel;
class UPanelWidget;
class UCSKDoughProfileButtonWidget;
class UCSKLobbySlotWidget;

/**
 * 로비. 싱글이든 멀티든 이 화면 하나다 — 싱글은 봇으로, 멀티는 사람으로 자리가 채워질 뿐.
 * 레이아웃은 WBP_CSKLobby.
 *
 * 참가자 슬롯은 UI 격자이고, 반죽은 그 칸 자리에 맞춰 레벨에 실제로 서는 3D다
 * (ACSKLobbyPreviewStage). 렌더타겟을 거치지 않으므로 화질이 인게임 렌더 그대로다.
 * 이 위젯은 매 프레임 각 슬롯 칸의 화면 좌표를 재서 넘기기만 하고, 격자의 크기·비율·배치는
 * 전적으로 WBP_CSKLobby가 정한다 — 칸을 어디에 어떻게 두든 반죽이 따라온다.
 *
 * 이 위젯은 명단을 GameMode에 넘기기만 하고 월드를 직접 뒤지지 않으므로,
 * 나중에 로비가 별도 레벨로 분리돼도 그대로 옮겨간다.
 */
UCLASS()
class CHAPSALKING_API UCSKLobbyWidget : public UCSKMenuWidget
{
	GENERATED_BODY()

public:
	// 프로필 칸이 호출한다
	void SelectDough(EDoughCharacterType NewType);

	// 로비 화면에 들어오고 나갈 때 3D 반죽을 켜고 끈다
	virtual void NativeOnScreenActivated() override;
	virtual void NativeOnScreenDeactivated() override;

protected:
	virtual void NativeConstruct() override;

	// 슬롯 칸의 화면 자리를 3D 반죽에 먹인다. 레이아웃은 창 크기·DPI·애니메이션으로
	// 언제든 움직이므로 한 번 재고 마는 대신 매 프레임 따라간다
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void BuildProfileButtons();
	void BuildSlotWidgets();

	// 선택 표시·봇 인원 텍스트·3D 프리뷰를 현재 GameInstance 상태에 맞춘다
	void Refresh();

	UFUNCTION()
	void HandleBotCountDecrease();

	UFUNCTION()
	void HandleBotCountIncrease();

	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleBackClicked();

	// 칸 수만큼 코드가 찍어내는 위젯들 — WBP_CSKLobby 디폴트에서 지정한다
	UPROPERTY(EditDefaultsOnly, Category = "CSK|Menu")
	TSubclassOf<UCSKDoughProfileButtonWidget> ProfileButtonClass;

	UPROPERTY(EditDefaultsOnly, Category = "CSK|Menu")
	TSubclassOf<UCSKLobbySlotWidget> SlotWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> SlotPanel;

	// 프로필 칸이 들어갈 컨테이너. WrapBox든 HorizontalBox든 WBP가 정한다
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> ProfilePanel;

	// 오른쪽 상세 패널 — 지금 고른 반죽을 보여 준다
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SelectedNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SelectedDescText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BotCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BotCountDecreaseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BotCountIncreaseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

	UPROPERTY()
	TArray<TObjectPtr<UCSKDoughProfileButtonWidget>> ProfileButtons;

	UPROPERTY()
	TArray<TObjectPtr<UCSKLobbySlotWidget>> SlotWidgets;

	// 로비 화면일 때만 참이다. 3D 반죽이 서 있는 동안에만 자리를 갱신한다
	bool bStageActive = false;
};
