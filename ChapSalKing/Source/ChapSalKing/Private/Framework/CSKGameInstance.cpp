#include "Framework/CSKGameInstance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Framework/CSKProjectSettingsData.h"
#include "Dough/DoughDataRegistry.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"

void UCSKGameInstance::Init()
{
	Super::Init();

	ApplyFirstRunScalability();
}

void UCSKGameInstance::ApplyFirstRunScalability()
{
	// 에디터의 그래픽 등급은 에디터 뷰포트 설정이 관리한다. PIE마다 벤치마크가 돌면 그걸 덮어쓴다
	if (GIsEditor)
	{
		return;
	}

	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();

	// 벤치마크를 한 번도 안 돌렸으면 결과가 음수(-1)로 남아 있다 — 이게 첫 실행의 표시다
	if (UserSettings->GetLastGPUBenchmarkResult() >= 0.0f)
	{
		return;
	}

	UserSettings->RunHardwareBenchmark();
	UserSettings->ApplyHardwareBenchmarkResults();
}

UCSKProjectSettingsData* UCSKGameInstance::GetProjectSettings(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
		: nullptr;
	if (!World)
	{
		return nullptr;
	}

	const UCSKGameInstance* GameInstance = World->GetGameInstance<UCSKGameInstance>();
	return GameInstance ? GameInstance->ProjectSettingsData : nullptr;
}

UCSKProjectSettingsData* UCSKGameInstance::GetProjectSettingsData() const
{
	return ProjectSettingsData;
}

bool UCSKGameInstance::IsDrawDebugEnabled(const UObject* WorldContextObject)
{
    const UCSKProjectSettingsData* Settings = GetProjectSettings(WorldContextObject);
    return Settings && Settings->bDrawDebug;
}

void UCSKGameInstance::SetSelectedDoughType(EDoughCharacterType NewType)
{
	SelectedDoughType = NewType;
}

void UCSKGameInstance::SetBotCount(int32 NewBotCount)
{
	BotCount = FMath::Clamp(NewBotCount, 0, FMath::Max(GetMaxPlayerCount() - 1, 0));
}

int32 UCSKGameInstance::GetMaxPlayerCount() const
{
	if (!ProjectSettingsData || !ProjectSettingsData->DoughDataRegistry)
	{
		return 0;
	}

	return ProjectSettingsData->DoughDataRegistry->GetAllCharacterTypes().Num();
}

TArray<EDoughCharacterType> UCSKGameInstance::BuildMatchRoster() const
{
	TArray<EDoughCharacterType> Roster;
	if (!ProjectSettingsData || !ProjectSettingsData->DoughDataRegistry)
	{
		return Roster;
	}

	// 반죽은 중복되지 않는다 — 플레이어가 고른 색을 뺀 나머지를 봇에게 순서대로 배정
	TArray<EDoughCharacterType> AvailableTypes =
		ProjectSettingsData->DoughDataRegistry->GetAllCharacterTypes();
	AvailableTypes.Remove(SelectedDoughType);

	Roster.Add(SelectedDoughType);

	const int32 SpawnableBotCount = FMath::Min(BotCount, AvailableTypes.Num());
	for (int32 Index = 0; Index < SpawnableBotCount; ++Index)
	{
		Roster.Add(AvailableTypes[Index]);
	}

	return Roster;
}

void UCSKGameInstance::TravelToMatch()
{
	if (!ProjectSettingsData || ProjectSettingsData->MatchSettings.MatchLevel.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] TravelToMatch: MatchSettings.MatchLevel 미설정"));
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, ProjectSettingsData->MatchSettings.MatchLevel);
}
