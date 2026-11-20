#include "Framework/CSKMenuGameMode.h"

#include "Dough/DoughDataRegistry.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "UI/Menu/CSKMenuHUD.h"
#include "UI/Menu/CSKLobbyPreviewStage.h"

#include "Engine/World.h"

ACSKMenuGameMode::ACSKMenuGameMode()
{
	HUDClass = ACSKMenuHUD::StaticClass();

	// 조작할 캐릭터가 없다. 폰을 스폰하면 메뉴 뒤에서 빈 캐릭터가 돌아다닌다
	DefaultPawnClass = nullptr;
}

ACSKLobbyPreviewStage* ACSKMenuGameMode::GetOrCreatePreviewStage()
{
	if (PreviewStage)
	{
		return PreviewStage;
	}

	// 스탠드가 월드 절대 좌표로 서므로 스테이지 자신의 자리는 의미가 없다
	PreviewStage = GetWorld()->SpawnActor<ACSKLobbyPreviewStage>(
		ACSKLobbyPreviewStage::StaticClass(), FTransform::Identity);

	return PreviewStage;
}

void ACSKMenuGameMode::SetLobbyPreviewVisible(bool bVisible)
{
	GetOrCreatePreviewStage()->SetStageVisible(bVisible);
}

void ACSKMenuGameMode::AlignLobbyPreview(
	APlayerController* PlayerController,
	const TArray<FCSKLobbySlotViewportRect>& SlotRects)
{
	GetOrCreatePreviewStage()->AlignToSlots(PlayerController, SlotRects);
}

void ACSKMenuGameMode::ApplyLobbyRoster(const TArray<EDoughCharacterType>& Roster)
{
	ACSKLobbyPreviewStage* Stage = GetOrCreatePreviewStage();

	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	const UDoughDataRegistry* Registry = Settings ? Settings->DoughDataRegistry : nullptr;
	if (!Registry)
	{
		return;
	}

	TArray<UDoughData*> RosterDoughData;
	RosterDoughData.Reserve(Roster.Num());
	for (const EDoughCharacterType CharacterType : Roster)
	{
		RosterDoughData.Add(Registry->FindByType(CharacterType));
	}

	Stage->ApplyRoster(RosterDoughData);
}
