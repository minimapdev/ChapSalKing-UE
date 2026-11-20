#pragma once

#include "CoreMinimal.h"
#include "UI/Menu/CSKMenuWidget.h"
#include "Dough/DoughTypes.h"
#include "CSKDoughProfileButtonWidget.generated.h"

class UButton;
class UImage;
class UWidget;
class UDoughData;
class UCSKLobbyWidget;

/**
 * 로비 반죽 선택 팔레트의 한 칸. 반죽 하나를 대표한다. 레이아웃은 WBP_CSKDoughProfile.
 *
 * 반죽 종류가 곧 색이라 칸은 아이콘이 아니라 그 반죽의 몸 색을 칠한 스와치다.
 * 칸은 레지스트리에 등록된 반죽 수만큼 UCSKLobbyWidget이 만들어 넣는다 —
 * 반죽이 늘면 칸도 따라 는다. 칸 크기·여백·모양은 WBP가 정하고, 코드는 스와치 색과
 * 선택 표시만 건드린다.
 */
UCLASS()
class CHAPSALKING_API UCSKDoughProfileButtonWidget : public UCSKMenuWidget
{
	GENERATED_BODY()

public:
	void InitProfile(UDoughData* InDoughData, UCSKLobbyWidget* InOwnerLobby);
	void SetSelected(bool bInSelected);

	EDoughCharacterType GetCharacterType() const;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ProfileButton;

	// 이 반죽의 몸 색을 칠할 자리. 브러시는 흰색이어야 색이 그대로 나온다
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SwatchImage;

	// 지금 고른 칸에만 보이는 표시. 테두리든 체크든 WBP가 한 덩어리로 묶어 두면
	// 코드는 켜고 끄기만 한다
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> SelectedMarker;

	UPROPERTY()
	TObjectPtr<UDoughData> DoughData;

	UPROPERTY()
	TObjectPtr<UCSKLobbyWidget> OwnerLobby;
};
