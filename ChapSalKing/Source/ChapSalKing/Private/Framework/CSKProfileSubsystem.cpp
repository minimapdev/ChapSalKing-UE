#include "Framework/CSKProfileSubsystem.h"

#include "Framework/CSKProfileSaveGame.h"

#include "Kismet/GameplayStatics.h"

namespace CSKProfile
{
	const FString SlotName = TEXT("CSKProfile");
	constexpr int32 UserIndex = 0;
}

void UCSKProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 첫 실행이면 슬롯이 없다 — 빈 기록으로 시작하고, 처음 기록할 때 슬롯이 생긴다
	Profile = UGameplayStatics::DoesSaveGameExist(CSKProfile::SlotName, CSKProfile::UserIndex)
		? CastChecked<UCSKProfileSaveGame>(UGameplayStatics::LoadGameFromSlot(CSKProfile::SlotName, CSKProfile::UserIndex))
		: CastChecked<UCSKProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(UCSKProfileSaveGame::StaticClass()));
}

bool UCSKProfileSubsystem::HasSeenIntro() const
{
	return Profile->bIntroSeen;
}

void UCSKProfileSubsystem::MarkIntroSeen()
{
	if (Profile->bIntroSeen)
	{
		return;
	}

	Profile->bIntroSeen = true;
	Save();
}

void UCSKProfileSubsystem::Save() const
{
	UGameplayStatics::SaveGameToSlot(Profile, CSKProfile::SlotName, CSKProfile::UserIndex);
}
