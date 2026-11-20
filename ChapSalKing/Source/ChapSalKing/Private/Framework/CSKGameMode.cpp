#include "Framework/CSKGameMode.h"

#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKEatableComponent.h"
#include "Framework/CSKPlayerState.h"
#include "Framework/CSKPlayerController.h"
#include "Framework/CSKGameState.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "UI/CSKHUD.h"
#include "Props/King/StatuePedestal.h"
#include "Props/Mortar/PestleItem.h"
#include "Props/Pouch/LuckyPouch.h"
#include "Framework/CSKPouchDropZone.h"
#include "Ingredient/IngredientData.h"
#include "Dough/DoughDataRegistry.h"
#include "AI/CSKBotAIController.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

ACSKGameMode::ACSKGameMode()
{
	GameStateClass = ACSKGameState::StaticClass();
	PlayerControllerClass = ACSKPlayerController::StaticClass();
	PlayerStateClass = ACSKPlayerState::StaticClass();
	HUDClass = ACSKHUD::StaticClass();
}

void ACSKGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (ACSKGameState* CSKGameState = GetGameState<ACSKGameState>())
	{
		ChapSalKingChangedHandle = CSKGameState->OnChapSalKingChanged.AddUObject(
			this, &ACSKGameMode::HandleChapSalKingChanged);
	}

	SpawnBots();
	StartMatch();
}

void ACSKGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ACSKGameState* CSKGameState = GetGameState<ACSKGameState>())
	{
		CSKGameState->OnChapSalKingChanged.Remove(ChapSalKingChangedHandle);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

APawn* ACSKGameMode::SpawnDefaultPawnAtTransform_Implementation(
	AController* NewPlayer, const FTransform& SpawnTransform)
{
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass)
	{
		return nullptr;
	}

	const UCSKGameInstance* GameInstance = GetGameInstance<UCSKGameInstance>();
	if (!GameInstance)
	{
		return nullptr;
	}

	return SpawnDoughPawn(PawnClass, SpawnTransform, GameInstance->SelectedDoughType);
}

ACSKPlayerCharacter* ACSKGameMode::SpawnDoughPawn(
	UClass* PawnClass, const FTransform& SpawnTransform, EDoughCharacterType DoughType)
{
	UWorld* World = GetWorld();
	if (!World || !PawnClass)
	{
		return nullptr;
	}

	ACSKPlayerCharacter* Pawn = World->SpawnActorDeferred<ACSKPlayerCharacter>(
		PawnClass, SpawnTransform, nullptr, GetInstigator(),
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	if (!Pawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] SpawnDoughPawn: %s는 ACSKPlayerCharacter가 아닙니다"),
			*PawnClass->GetName());
		return nullptr;
	}

	// FinishSpawning이 BeginPlay를 돌리고, 그 안에서 InitDoughData가 이 값을 읽는다
	Pawn->DoughType = DoughType;

	Pawn->FinishSpawning(SpawnTransform);
	return Pawn;
}

void ACSKGameMode::SpawnBots()
{
	UWorld* World = GetWorld();
	const UCSKGameInstance* GameInstance = GetGameInstance<UCSKGameInstance>();
	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	if (!World || !GameInstance || !Settings || !Settings->DoughDataRegistry)
	{
		return;
	}

	const FCSKBotSpawnSettings& BotSettings = Settings->BotSpawnSettings;
	if (!BotSettings.BotPawnClass || !BotSettings.BotControllerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] SpawnBots: BotPawnClass/BotControllerClass 미설정"));
		return;
	}

	// 로비 프리뷰가 보여준 것과 같은 명단 — 0번은 플레이어 몫이라 건너뛴다
	const TArray<EDoughCharacterType> Roster = GameInstance->BuildMatchRoster();
	const int32 BotCount = FMath::Max(Roster.Num() - 1, 0);
	if (BotCount < GameInstance->BotCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] SpawnBots: 반죽이 %d종뿐이라 봇을 %d명만 스폰합니다"),
			Roster.Num(), BotCount);
	}

	int32 PlayerStartCount = 0;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		++PlayerStartCount;
	}

	if (PlayerStartCount < BotCount + 1)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CSK] SpawnBots: PlayerStart가 %d개뿐입니다 (필요 %d개) — 시작 지점이 겹칩니다"),
			PlayerStartCount, BotCount + 1);
	}

	for (int32 Index = 0; Index < BotCount; ++Index)
	{
		// 시작 지점 점유 판정을 GameModeBase에 맡기려면 컨트롤러가 먼저 있어야 한다
		ACSKBotAIController* BotController =
			World->SpawnActor<ACSKBotAIController>(BotSettings.BotControllerClass);
		if (!BotController)
		{
			continue;
		}

		AActor* StartSpot = FindPlayerStart(BotController);
		if (!StartSpot)
		{
			UE_LOG(LogTemp, Warning, TEXT("[CSK] SpawnBots: PlayerStart가 레벨에 없음"));
			BotController->Destroy();
			return;
		}

		ACSKPlayerCharacter* BotPawn =
			SpawnDoughPawn(BotSettings.BotPawnClass, StartSpot->GetActorTransform(), Roster[Index + 1]);
		if (!BotPawn)
		{
			BotController->Destroy();
			continue;
		}

		BotController->Possess(BotPawn);
	}
}

void ACSKGameMode::RegisterPedestal(AStatuePedestal* Pedestal)
{
	if (Pedestal)
	{
		Pedestals.AddUnique(Pedestal);
	}
}

void ACSKGameMode::UnregisterPedestal(AStatuePedestal* Pedestal)
{
	Pedestals.Remove(Pedestal);
}

void ACSKGameMode::HandleChapSalKingChanged(ACSKPlayerCharacter* NewKing)
{
	ACSKGameState* CSKGameState = GetGameState<ACSKGameState>();

	if (NewKing != nullptr)
	{
		if (CSKGameState)
		{
			CSKGameState->BroadcastMatchEvent(
				FCSKMatchEventInfo(ECSKMatchEventType::KingAppeared, NewKing->GetPlayerState()));
		}

		for (AStatuePedestal* Pedestal : Pedestals)
		{
			if (Pedestal)
			{
				Pedestal->RaiseStatue(NewKing);
			}
		}
		return;
	}

	for (AStatuePedestal* Pedestal : Pedestals)
	{
		if (Pedestal)
		{
			Pedestal->ClearStatue();
		}
	}

	if (CSKGameState)
	{
		CSKGameState->BroadcastMatchEvent(FCSKMatchEventInfo(ECSKMatchEventType::KingEnded));

		if (CSKGameState->IsMatchFinished())
		{
			return;
		}
	}

	// 킹 종료 — 달 절구에 꽂힌 공이를 뽑아 복주머니에 숨긴다. 다음 웨이브에서 다시 나온다
	ScheduleNextPestleWave();
}

void ACSKGameMode::StartMatch()
{
	float Duration = 300.0f;
	if (const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this))
	{
		Duration = Settings->MatchSettings.MatchDuration;
	}

	if (ACSKGameState* CSKGameState = GetGameState<ACSKGameState>())
	{
		CSKGameState->StartMatch(Duration);
	}

	ScheduleNextPestleWave();

	// 상시 낙하는 웨이브와 무관하게 매치 내내 돈다 — 지상 재료가 없으므로 이것이 보급의 전부다
	if (const UCSKProjectSettingsData* PouchOwner = UCSKGameInstance::GetProjectSettings(this))
	{
		const float Interval = PouchOwner->PouchDropSettings.PouchInterval;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				PouchIntervalTimerHandle, this, &ACSKGameMode::DropSinglePouch, Interval, true, Interval);
		}
	}

	if (Duration > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				MatchTimerHandle, this, &ACSKGameMode::EndMatch, Duration, false);
		}
	}
}

void ACSKGameMode::EndMatch()
{
	FinalizeMatch(nullptr);
}

void ACSKGameMode::TryEndMatchFromKingCapture(ACSKPlayerCharacter* KingCharacter)
{
	if (!KingCharacter || !AreAllOpponentsStuckToKing(KingCharacter))
	{
		return;
	}

	APlayerState* WinnerPlayerState = KingCharacter->GetPlayerState();
	if (!WinnerPlayerState)
	{
		return;
	}

	FinalizeMatch(WinnerPlayerState);
}

bool ACSKGameMode::AreAllOpponentsStuckToKing(const ACSKPlayerCharacter* KingCharacter) const
{
	const ACSKGameState* CSKGameState = GetGameState<ACSKGameState>();
	if (!KingCharacter || !CSKGameState || !CSKGameState->IsMatchInProgress()
		|| CSKGameState->GetChapSalKing() != KingCharacter)
	{
		return false;
	}

	const APlayerState* KingPlayerState = KingCharacter->GetPlayerState();
	int32 OpponentCount = 0;

	for (APlayerState* PlayerState : CSKGameState->PlayerArray)
	{
		if (!PlayerState || PlayerState->IsOnlyASpectator() || PlayerState == KingPlayerState)
		{
			continue;
		}

		++OpponentCount;

		const ACSKPlayerCharacter* OpponentCharacter =
			Cast<ACSKPlayerCharacter>(PlayerState->GetPawn());
		if (!OpponentCharacter)
		{
			return false;
		}

		const UCSKEatableComponent* EatableComponent =
			OpponentCharacter->GetEatableComponent();
		if (!EatableComponent
			|| EatableComponent->GetEatenState() != ECSKEatenState::StuckToKing
			|| !EatableComponent->IsBeingEatenBy(KingCharacter))
		{
			return false;
		}
	}

	return OpponentCount > 0;
}

void ACSKGameMode::FinalizeMatch(APlayerState* ForcedWinner)
{
	UWorld* World = GetWorld();
	ACSKGameState* CSKGameState = GetGameState<ACSKGameState>();
	if (!World || !CSKGameState || CSKGameState->IsMatchFinished())
	{
		return;
	}

	World->GetTimerManager().ClearTimer(MatchTimerHandle);

	TArray<FCSKPlayerScoreEntry> Results = CSKGameState->GetPlayerScores();
	if (ForcedWinner)
	{
		const int32 WinnerIndex = Results.IndexOfByPredicate(
			[ForcedWinner](const FCSKPlayerScoreEntry& Entry)
			{
				return Entry.PlayerState == ForcedWinner;
			});

		if (WinnerIndex > 0)
		{
			const FCSKPlayerScoreEntry WinnerEntry = Results[WinnerIndex];
			Results.RemoveAt(WinnerIndex);
			Results.Insert(WinnerEntry, 0);
		}
	}

	CSKGameState->FinishMatch(Results);

	for (TActorIterator<AAIController> It(World); It; ++It)
	{
		It->StopMovement();
		if (UBrainComponent* Brain = It->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("MatchEnded"));
		}
	}
}

void ACSKGameMode::ScheduleNextPestleWave()
{
	UWorld* World = GetWorld();
	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	if (!World || !Settings)
	{
		return;
	}

	// 카운트다운은 예고다. 이 시간 동안에도 상시 낙하가 계속 돌기 때문에 멍하니 기다리지 않는다
	const float LeadTime = Settings->PestleSpawnSettings.SpawnCountdown;
	World->GetTimerManager().SetTimer(
		PestleWaveTimerHandle, this, &ACSKGameMode::DropPestleWave, LeadTime, false);
}

void ACSKGameMode::DropPestleWave()
{
	UWorld* World = GetWorld();
	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	if (!World || !Settings)
	{
		return;
	}

	const FCSKPouchDropSettings& DropSettings = Settings->PouchDropSettings;

	// 절구공이는 매치 내내 단일 액터로 유지된다 — 없으면 한 번만 만들고, 이후로는 숨겼다 드러낸다
	APestleItem* Pestle = FindPestleInWorld();
	if (!Pestle)
	{
		if (!Settings->PestleSpawnSettings.PestleClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[CSK] DropPestleWave: PestleClass 미설정"));
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Pestle = World->SpawnActor<APestleItem>(
			Settings->PestleSpawnSettings.PestleClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	if (!Pestle)
	{
		return;
	}

	Pestle->StashIntoPouch();

	// 어느 주머니에 들었는지는 뽑기로 정한다. 겉모습이 같으므로 아무도 구분할 수 없다
	const int32 WaveCount = FMath::Max(DropSettings.PestleWaveCount, 1);
	const int32 PestleIndex = FMath::RandRange(0, WaveCount - 1);

	TArray<FVector> TakenLocations;
	TakenLocations.Reserve(WaveCount);

	bool bPestlePlaced = false;
	for (int32 Index = 0; Index < WaveCount; ++Index)
	{
		FVector LandingLocation;
		if (!FindDropLocation(DropSettings, TakenLocations, DropSettings.MinWaveSpacing, LandingLocation))
		{
			continue;
		}
		TakenLocations.Add(LandingLocation);

		// 절구공이 자리를 못 잡으면 매치 루프가 멈춘다 — 마지막 주머니에라도 반드시 넣는다
		const bool bIsPestlePouch = (Index == PestleIndex) || (!bPestlePlaced && Index == WaveCount - 1);
		SpawnPouchAt(DropSettings, LandingLocation, bIsPestlePouch ? Pestle : nullptr);
		bPestlePlaced |= bIsPestlePouch;
	}

	if (!bPestlePlaced)
	{
		// 존이 좁거나 네비메시가 없어 한 자리도 못 찾았다. 절구공이가 영영 안 나오면 안 되므로
		// 원래 있던 자리에 그대로 드러낸다
		UE_LOG(LogTemp, Warning,
			TEXT("[CSK] DropPestleWave: 낙하 지점을 찾지 못했습니다 — PouchDropZone 반경이나 네비메시를 확인하세요"));
		Pestle->RevealAt(Pestle->GetActorLocation());
	}

	if (ACSKGameState* CSKGameState = GetGameState<ACSKGameState>())
	{
		CSKGameState->BroadcastMatchEvent(FCSKMatchEventInfo(ECSKMatchEventType::PestleRespawned));
	}
}

void ACSKGameMode::DropSinglePouch()
{
	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	if (!Settings)
	{
		return;
	}

	if (const ACSKGameState* CSKGameState = GetGameState<ACSKGameState>())
	{
		if (CSKGameState->IsMatchFinished())
		{
			return;
		}
	}

	const FCSKPouchDropSettings& DropSettings = Settings->PouchDropSettings;

	FVector LandingLocation;
	if (!FindDropLocation(DropSettings, TArray<FVector>(), 0.0f, LandingLocation))
	{
		return;   // 이번 낙하는 거른다 — 다음 주기에 다시 기회
	}

	SpawnPouchAt(DropSettings, LandingLocation, nullptr);
}

ALuckyPouch* ACSKGameMode::SpawnPouchAt(
	const FCSKPouchDropSettings& DropSettings,
	const FVector& LandingLocation,
	APestleItem* PestleToHide)
{
	UWorld* World = GetWorld();
	if (!World || !DropSettings.PouchClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] SpawnPouchAt: PouchClass 미설정"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ALuckyPouch* Pouch = World->SpawnActor<ALuckyPouch>(
		DropSettings.PouchClass,
		LandingLocation + FVector(0.0f, 0.0f, DropSettings.DropHeight),
		FRotator::ZeroRotator, SpawnParams);

	if (!Pouch)
	{
		return nullptr;
	}

	UIngredientData* Ingredient = nullptr;
	bool bIsBomb = false;
	if (!PestleToHide)
	{
		RollLoot(DropSettings, Ingredient, bIsBomb);
	}

	Pouch->StartDrop(LandingLocation, Ingredient, bIsBomb, PestleToHide);

	if (ACSKGameState* CSKGameState = GetGameState<ACSKGameState>())
	{
		CSKGameState->BroadcastMatchEvent(FCSKMatchEventInfo(ECSKMatchEventType::PouchDropped));
	}

	return Pouch;
}

void ACSKGameMode::RollLoot(
	const FCSKPouchDropSettings& DropSettings, UIngredientData*& OutIngredient, bool& bOutIsBomb) const
{
	OutIngredient = nullptr;
	bOutIsBomb = false;

	int32 TotalWeight = 0;
	for (const FCSKPouchLootEntry& Entry : DropSettings.LootTable)
	{
		TotalWeight += FMath::Max(Entry.Weight, 0);
	}

	if (TotalWeight <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] RollLoot: LootTable이 비었거나 가중치가 전부 0입니다"));
		return;
	}

	int32 Roll = FMath::RandRange(0, TotalWeight - 1);
	for (const FCSKPouchLootEntry& Entry : DropSettings.LootTable)
	{
		Roll -= FMath::Max(Entry.Weight, 0);
		if (Roll >= 0)
		{
			continue;
		}

		if (Entry.LootType == ECSKPouchLootType::Bomb)
		{
			bOutIsBomb = true;
		}
		else
		{
			OutIngredient = Entry.Ingredient;
		}
		return;
	}
}

ACSKPouchDropZone* ACSKGameMode::PickRandomDropZone() const
{
	TArray<ACSKPouchDropZone*> Zones;
	for (TActorIterator<ACSKPouchDropZone> It(GetWorld()); It; ++It)
	{
		Zones.Add(*It);
	}

	if (Zones.Num() == 0)
	{
		return nullptr;   // 존이 없으면 아레나 전역을 쓴다
	}

	return Zones[FMath::RandRange(0, Zones.Num() - 1)];
}

bool ACSKGameMode::FindDropLocation(
	const FCSKPouchDropSettings& DropSettings,
	const TArray<FVector>& TakenLocations,
	float MinSeparation,
	FVector& OutLocation) const
{
	UWorld* World = GetWorld();
	UNavigationSystemV1* NavSystem = World ? UNavigationSystemV1::GetCurrent(World) : nullptr;
	if (!NavSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] FindDropLocation: NavigationSystem 없음 (NavMesh 미배치?)"));
		return false;
	}

	const ACSKPouchDropZone* Zone = PickRandomDropZone();

	for (int32 Try = 0; Try < 10; ++Try)
	{
		FNavLocation RandomLocation;
		const bool bFound = Zone
			? NavSystem->GetRandomPointInNavigableRadius(Zone->GetActorLocation(), Zone->GetRadius(), RandomLocation)
			: NavSystem->GetRandomPoint(RandomLocation);

		// 고지대는 안전하되 무자원으로 남긴다
		if (!bFound || RandomLocation.Location.Z > DropSettings.MaxDropZ)
		{
			continue;
		}

		// 구 바닥이 지면에 닿아 항상 블로킹되지 않도록 반경만큼 띄워서 검사
		const FVector ClearanceCenter =
			RandomLocation.Location + FVector(0.0f, 0.0f, DropSettings.DropClearanceRadius + 10.0f);
		if (World->OverlapBlockingTestByChannel(
				ClearanceCenter, FQuat::Identity, ECC_WorldStatic,
				FCollisionShape::MakeSphere(DropSettings.DropClearanceRadius)))
		{
			continue;
		}

		// 이번 웨이브에서 이미 잡은 자리와 겹치면 분산이 무너진다
		bool bTooClose = false;
		for (const FVector& Taken : TakenLocations)
		{
			if (FVector::Dist2D(Taken, RandomLocation.Location) < MinSeparation)
			{
				bTooClose = true;
				break;
			}
		}

		// 이미 땅에 있는 주머니와도 떨어져 있어야 한다
		if (!bTooClose)
		{
			for (TActorIterator<ALuckyPouch> It(World); It; ++It)
			{
				if (FVector::Dist2D(It->GetLandingLocation(), RandomLocation.Location) < DropSettings.MinDropSpacing)
				{
					bTooClose = true;
					break;
				}
			}
		}

		if (!bTooClose)
		{
			OutLocation = RandomLocation.Location;
			return true;
		}
	}

	return false;
}

APestleItem* ACSKGameMode::FindPestleInWorld() const
{
	for (TActorIterator<APestleItem> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

