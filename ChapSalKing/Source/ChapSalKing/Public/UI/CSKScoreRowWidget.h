#pragma once

#include "CoreMinimal.h"
#include "UI/CSKUserWidget.h"
#include "Framework/CSKGameState.h"
#include "CSKScoreRowWidget.generated.h"

class UTextBlock;
class UImage;
class UPanelWidget;
class APlayerController;
class APlayerState;

/**
 * 순위표 한 행. 스코어보드와 결과 화면이 생성·재사용한다.
 *
 * 요구 위젯: RankText / NameText / ScoreText (TextBlock), KingIcon (Image)
 */
UCLASS(Abstract)
class CHAPSALKING_API UCSKScoreRowWidget : public UCSKUserWidget
{
	GENERATED_BODY()

public:
	void SetEntry(const FCSKPlayerScoreEntry& Entry, int32 Rank, bool bIsLocalPlayer, bool bIsKing);

	/**
	 * 컨테이너를 Entries로 채운다. 기존 자식을 재사용하고 모자란 만큼만 새로 만든다.
	 * MaxRows가 0이면 전원 표시. KingPlayerState는 null 가능(결과 화면 등).
	 */
	static void PopulateRows(
		UPanelWidget* Container,
		TSubclassOf<UCSKScoreRowWidget> RowWidgetClass,
		APlayerController* OwningPlayer,
		const TArray<FCSKPlayerScoreEntry>& Entries,
		const APlayerState* LocalPlayerState,
		const APlayerState* KingPlayerState,
		int32 MaxRows);

protected:
	// 하이라이트·배경 등 스타일은 BP에서. UIStyleData의 LocalPlayerColor를 쓴다
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Score")
	void BP_OnEntryUpdated(bool bIsLocalPlayer, bool bIsKing);

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Score", meta = (BindWidget))
	TObjectPtr<UTextBlock> RankText;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Score", meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Score", meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(BlueprintReadOnly, Category = "CSK|Score", meta = (BindWidget))
	TObjectPtr<UImage> KingIcon;
};
