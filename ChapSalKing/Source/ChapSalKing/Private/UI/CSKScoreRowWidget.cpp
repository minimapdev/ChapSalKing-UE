#include "UI/CSKScoreRowWidget.h"

#include "UI/CSKUIStyleData.h"

#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

void UCSKScoreRowWidget::SetEntry(
	const FCSKPlayerScoreEntry& Entry, int32 Rank, bool bIsLocalPlayer, bool bIsKing)
{
	RankText->SetText(FText::AsNumber(Rank));
	NameText->SetText(FText::FromString(Entry.PlayerName));
	ScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d g"), Entry.Score)));

	KingIcon->SetVisibility(bIsKing ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	if (bIsKing)
	{
		if (const UCSKUIStyleData* Style = GetUIStyle())
		{
			if (Style->KingCrownIcon)
			{
				KingIcon->SetBrushFromTexture(Style->KingCrownIcon);
			}
		}
	}

	BP_OnEntryUpdated(bIsLocalPlayer, bIsKing);
}

void UCSKScoreRowWidget::PopulateRows(
	UPanelWidget* Container,
	TSubclassOf<UCSKScoreRowWidget> RowWidgetClass,
	APlayerController* OwningPlayer,
	const TArray<FCSKPlayerScoreEntry>& Entries,
	const APlayerState* LocalPlayerState,
	const APlayerState* KingPlayerState,
	int32 MaxRows)
{
	if (!Container || !RowWidgetClass || !OwningPlayer)
	{
		return;
	}

	const int32 VisibleCount = (MaxRows > 0) ? FMath::Min(MaxRows, Entries.Num()) : Entries.Num();

	// 모자란 만큼만 새로 만든다
	while (Container->GetChildrenCount() < VisibleCount)
	{
		UCSKScoreRowWidget* Row = CreateWidget<UCSKScoreRowWidget>(OwningPlayer, RowWidgetClass);
		if (!Row)
		{
			break;
		}
		Container->AddChild(Row);
	}

	for (int32 Index = 0; Index < Container->GetChildrenCount(); ++Index)
	{
		UCSKScoreRowWidget* Row = Cast<UCSKScoreRowWidget>(Container->GetChildAt(Index));
		if (!Row)
		{
			continue;
		}

		if (Index >= VisibleCount)
		{
			Row->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}

		const FCSKPlayerScoreEntry& Entry = Entries[Index];
		Row->SetVisibility(ESlateVisibility::HitTestInvisible);
		Row->SetEntry(
			Entry,
			Index + 1,
			Entry.PlayerState == LocalPlayerState,
			KingPlayerState != nullptr && Entry.PlayerState == KingPlayerState);
	}
}
