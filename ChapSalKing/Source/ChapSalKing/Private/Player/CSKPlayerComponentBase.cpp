#include "Player/CSKPlayerComponentBase.h"

#include "Player/CSKPlayerCharacter.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "Player/CSKPlayerStatData.h"

bool UCSKPlayerComponentBase::CacheOwnerCSKCharacter()
{
	OwnerCSKCharacter = Cast<ACSKPlayerCharacter>(GetOwner());
	checkf(OwnerCSKCharacter, TEXT("%s는 ACSKPlayerCharacter에만 부착할 수 있습니다."), *GetNameSafe(GetClass()));
	return OwnerCSKCharacter != nullptr;
}

void UCSKPlayerComponentBase::InitStatData()
{
	PlayerStatData = GetPlayerStatData();
	checkf(PlayerStatData, TEXT("ProjectSettings의 PlayerStatData가 설정되지 않았습니다."));
}

UCSKProjectSettingsData* UCSKPlayerComponentBase::GetProjectSettings() const
{
	return UCSKGameInstance::GetProjectSettings(this);
}

UCSKPlayerStatData* UCSKPlayerComponentBase::GetPlayerStatData() const
{
	const UCSKProjectSettingsData* Settings = GetProjectSettings();
	return Settings ? Settings->PlayerStatData : nullptr;
}
