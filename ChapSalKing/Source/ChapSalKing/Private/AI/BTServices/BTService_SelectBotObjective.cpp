#include "AI/BTServices/BTService_SelectBotObjective.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"

#include "AI/CSKBotAIController.h"
#include "AI/CSKBotAIConfigData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerScaleComponent.h"
#include "Player/Components/CSKPlayerIngredientComponent.h"
#include "Player/Components/CSKPlayerStatusComponent.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKGameState.h"
#include "Dough/DoughBlob.h"
#include "Props/Pouch/LuckyPouch.h"
#include "Props/Mortar/PestleItem.h"
#include "Props/Cauldron/Cauldron.h"
#include "Props/Field/SmokeCloud.h"

namespace
{
	static const float FleeCandidateAngles[] = { 0.f, 30.f, -30.f, 60.f, -60.f, 90.f, -90.f, 135.f, -135.f };
	constexpr float FleeBuffer = 150.f;
	constexpr float EdgePenaltyWeight = 2.0f;
	constexpr float ReachAcceptRadius = 150.f;
	constexpr int32 ObjectiveCount = CSKBotObjectiveCount;
	static const FVector NavProjectExtent(300.f, 300.f, 400.f);

	// 반경 내 조건 통과 최근접 액터. TODO: GameState에서 받아오도록
	template <typename T, typename FilterT>
	T* FindNearest(UWorld* World, const FVector& Origin, float Radius, FilterT Filter, float& OutDist)
	{
		T* Best = nullptr;
		float BestDist = Radius;
		for (TActorIterator<T> It(World); It; ++It)
		{
			T* Actor = *It;
			if (!Filter(Actor))
			{
				continue;
			}
			const float Dist = FVector::Dist2D(Origin, Actor->GetActorLocation());
			if (Dist < BestDist)
			{
				BestDist = Dist;
				Best = Actor;
			}
		}
		OutDist = BestDist;
		return Best;
	}
}

UBTService_SelectBotObjective::UBTService_SelectBotObjective()
{
	NodeName = TEXT("Select Bot Objective");
	bNotifyTick = true;
	bNotifyBecomeRelevant = true;
	Interval = 0.25f;
	RandomDeviation = 0.05f;
}

void UBTService_SelectBotObjective::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		KingActorKey.ResolveSelectedKey(*BBAsset);
		ObjectiveKey.ResolveSelectedKey(*BBAsset);
		MoveTargetKey.ResolveSelectedKey(*BBAsset);
		AttackTargetKey.ResolveSelectedKey(*BBAsset);
	}
}

uint16 UBTService_SelectBotObjective::GetInstanceMemorySize() const
{
	return sizeof(FObjectiveMemory);
}

void UBTService_SelectBotObjective::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	*reinterpret_cast<FObjectiveMemory*>(NodeMemory) = FObjectiveMemory();
}

void UBTService_SelectBotObjective::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ACSKBotAIController* AIController = Cast<ACSKBotAIController>(OwnerComp.GetAIOwner());
	ACSKPlayerCharacter* Bot = AIController ? Cast<ACSKPlayerCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Bot || !Blackboard || Bot->IsChapSalKing())
	{
		return;
	}

	const UCSKBotAIConfigData* Config = AIController->GetAIConfig();
	const FCSKBotPersonality& Personality = AIController->GetPersonality();
	FObjectiveMemory* Memory = reinterpret_cast<FObjectiveMemory*>(NodeMemory);
	UWorld* World = Bot->GetWorld();
	const FVector BotLocation = Bot->GetActorLocation();
	const float Now = World->GetTimeSeconds();

	// --- 입력 수집 ---
	ACSKPlayerCharacter* CurrentKing = Cast<ACSKPlayerCharacter>(Blackboard->GetValueAsObject(KingActorKey.SelectedKeyName));

	// 위협도 계산
	const float DangerRange = FMath::Lerp(Config->KingDangerRangeMin, Config->KingDangerRangeMax, Personality.Aggression);
	const float ThreatRange = (Memory->Current == ECSKBotObjective::Flee)
		? DangerRange * Config->DoughFleeExitMultiplier	// 도망 중이면 넓게
		: DangerRange;
	float Threat = 0.0f;

	if (CurrentKing)
	{
		const float DistToKing = FVector::Dist2D(BotLocation, CurrentKing->GetActorLocation());
		Threat = FMath::Clamp(1.0f - DistToKing / ThreatRange, 0.0f, 1.0f);	// 킹이 가까울수록 커짐

		// 연막 속 킹은 위협으로 인지 못 함
		if (Threat > 0.0f && ASmokeCloud::IsSightBlocked(World, BotLocation, CurrentKing->GetActorLocation(), true))
		{
			Threat = 0.0f;
		}
	}

	// 내 등수를 정규화
	float RankPressure = 0.0f;
	if (const ACSKGameState* GameState = World->GetGameState<ACSKGameState>())
	{
		const TArray<FCSKPlayerScoreEntry>& Entries = GameState->GetPlayerScores();
		if (Entries.Num() > 1)
		{
			const APlayerState* MyPlayerState = Bot->GetPlayerState();
			for (int32 Index = 0; Index < Entries.Num(); ++Index)
			{
				if (Entries[Index].PlayerState == MyPlayerState)
				{
					RankPressure = (float)Index / (Entries.Num() - 1);
					break;
				}
			}
		}
	}

	// 반죽 조각 탐색
	float JellyDist = 0.0f;
	ADoughBlob* Jelly = FindNearest<ADoughBlob>(World, BotLocation, Config->JellySearchRadius,
		[](ADoughBlob* Blob) { return IsValid(Blob); }, JellyDist);

	// 복주머니 탐색. 떨어지는 중인 것은 아직 주울 수 없으므로 제외한다
	float ItemDist = 0.0f;
	ALuckyPouch* Item = FindNearest<ALuckyPouch>(World, BotLocation, Config->IngredientSearchRadius,
		[](ALuckyPouch* InItem) { return IsValid(InItem) && !InItem->IsFalling(); }, ItemDist);

	// 전투 상대 탐색. 인지 차단 중엔 스킵, 연막 뒤 후보는 제외
	float FoeDist = 0.0f;
	ACSKPlayerCharacter* Foe = nullptr;
	if (!Bot->GetStatusComponent()->HasStatus(ECSKStatusType::VisionBlocked))
	{
		Foe = FindNearest<ACSKPlayerCharacter>(World, BotLocation, Config->FightSearchRadius,
			[Bot, World, &BotLocation](ACSKPlayerCharacter* Other)
			{
				return IsValid(Other) && Other != Bot && !Other->IsChapSalKing() && !Other->IsBeingEaten()
					&& !ASmokeCloud::IsSightBlocked(World, BotLocation, Other->GetActorLocation());
			}, FoeDist);
	}

	// --- 점수 계산 ---
	const float JellyProx = Jelly ? 1.0f - JellyDist / Config->JellySearchRadius : 0.0f;
	const float ItemProx = Item ? 1.0f - ItemDist / Config->IngredientSearchRadius : 0.0f;
	const float FoeProx = Foe ? 1.0f - FoeDist / Config->FightSearchRadius : 0.0f;

	// 체급 우위
	float SizeAdvantage = 0.5f;
	if (Foe)
	{
		const UCSKPlayerScaleComponent* MyScale = Bot->GetScaleComponent();
		const UCSKPlayerScaleComponent* FoeScale = Foe->GetScaleComponent();
		if (MyScale && FoeScale)
		{
			const float MyMass = MyScale->GetMass();
			const float FoeMass = FoeScale->GetMass();
			SizeAdvantage = MyMass / FMath::Max(MyMass + FoeMass, KINDA_SMALL_NUMBER);
		}
	}

	// 탄약 게이지가 찰수록 파밍 욕구 감소
	const UCSKPlayerIngredientComponent* IngredientComp = Bot->GetIngredientComponent();
	const float FarmNeed = IngredientComp ? 1.0f - IngredientComp->GetGaugeRatio() : 0.0f;

	// 꼴찌일수록 젤리 위험 회피를 무시
	const float RiskAversion = Config->JellyRiskAversion * (1.0f - RankPressure * Config->RankRiskTolerance);

	// 점수 배열 채우기. 해당 목표를 지금할 가치로 평가.
	float Scores[ObjectiveCount] = {};
	Scores[(uint8)ECSKBotObjective::Wander] = Config->WanderBaseScore;
	Scores[(uint8)ECSKBotObjective::Flee] = Threat * Threat * Config->FleeWeight * Config->RankFleeScale.LerpBy(RankPressure);

	Scores[(uint8)ECSKBotObjective::PickJelly] =
		JellyProx * (1.0f - Threat * RiskAversion) * Personality.Greed
		* Config->JellyWeight * Config->RankJellyScale.LerpBy(RankPressure);

	Scores[(uint8)ECSKBotObjective::Fight] =
		FoeProx * FMath::Pow(SizeAdvantage, Config->SizeAdvantagePower) * Personality.Aggression
		* (1.0f - Threat) * Config->FightWeight * Config->RankFightScale.LerpBy(RankPressure);

	Scores[(uint8)ECSKBotObjective::Farm] =
		FarmNeed * ItemProx * (1.0f - Threat) * Config->FarmWeight;

	// 절구공이 반응.(인지 지연 적용)
	APestleItem* Pestle = AIController->GetPestleItem();
	if (IsValid(Pestle) && !Pestle->IsPlanted())
	{
		if (Memory->PestleSeenTime < 0.0f)
		{
			Memory->PestleSeenTime = Now;
		}

		const float NoticeDelay = Config->PestleNoticeDelayRange.LerpBy(1.0f - Personality.Reflex);
		if (Now - Memory->PestleSeenTime >= NoticeDelay)
		{
			const float PestleProx = 1.0f - FMath::Clamp(FVector::Dist2D(BotLocation, Pestle->GetActorLocation()) / Config->PestleSearchRadius, 0.0f, 1.0f);
			Scores[(uint8)ECSKBotObjective::Pestle] = PestleProx * Personality.Ambition * Config->PestleWeight * Config->RankPestleScale.LerpBy(RankPressure);
		}
	}
	else
	{
		Memory->PestleSeenTime = -1.0f;
	}

	// --- 선택 + 전환 관성 ---
	ECSKBotObjective Best = ECSKBotObjective::Wander;
	for (uint8 Index = 0; Index < ObjectiveCount; ++Index)
	{
		// 점수 배열에서 최댓값을 목표로 선택.
		if (Scores[Index] > Scores[(uint8)Best])
		{
			Best = (ECSKBotObjective)Index;
		}
	}

	// 새 목표가 더 높아도 바로 안 바꿈.
	if (Best != Memory->Current)
	{
		const bool bCommitTimeHeld = (Now - Memory->LastSwitchTime) < Config->MinCommitSeconds;
		const bool bNotWorthSwitch = Scores[(uint8)Best] < Scores[(uint8)Memory->Current] * Config->CommitBonus;
		if (bCommitTimeHeld || bNotWorthSwitch)
		{
			Best = Memory->Current;
		}
		else
		{
			Memory->Current = Best;
			Memory->LastSwitchTime = Now;
			Memory->bHasMoveTarget = false;
		}
	}

	// 절구 공이 운반 중에는 관성 무시
	if (Bot->GetCarriedPestleItem())
	{
		if (Memory->Current != ECSKBotObjective::Pestle)
		{
			Memory->Current = ECSKBotObjective::Pestle;
			Memory->LastSwitchTime = Now;
			Memory->bHasMoveTarget = false;
		}
		Best = ECSKBotObjective::Pestle;
	}

	// --- 목표별 이동 지점 ---
	FVector MoveTarget = BotLocation;
	ACSKPlayerCharacter* FightTarget = nullptr;

	switch (Best)
	{
	case ECSKBotObjective::Flee:
		if (CurrentKing)
		{
			MoveTarget = ComputeFleePoint(*Bot, CurrentKing->GetActorLocation(),
				DangerRange * Config->DoughFleeExitMultiplier, *Config, *Memory);

			// 열린 가마솥이 킹보다 가까우면 그리로. 뚜껑이 닫히면 매 틱 재평가로 자연히 포기한다
			const float KingDist = FVector::Dist2D(BotLocation, CurrentKing->GetActorLocation());
			float BestCauldronDist = KingDist;
			for (ACauldron* Cauldron : AIController->GetCauldrons())
			{
				if (!IsValid(Cauldron) || !Cauldron->CanEnter(Bot))
				{
					continue;
				}
				const float Dist = FVector::Dist2D(BotLocation, Cauldron->GetActorLocation());
				if (Dist < BestCauldronDist)
				{
					BestCauldronDist = Dist;
					MoveTarget = Cauldron->GetActorLocation();
				}
			}
		}
		break;

	case ECSKBotObjective::PickJelly:
		if (Jelly)
		{
			MoveTarget = Jelly->GetActorLocation();
		}
		break;

	case ECSKBotObjective::Fight:
		if (Foe)
		{
			FightTarget = Foe;

			const bool bArmed = IngredientComp && IngredientComp->HasIngredient();
			MoveTarget = (bArmed && FoeDist <= Config->ShootRange * Config->ShootStandoffRatio) ? BotLocation : Foe->GetActorLocation();
		}
		break;

	case ECSKBotObjective::Farm:
		if (Item)
		{
			MoveTarget = Item->GetActorLocation();
		}
		break;

	case ECSKBotObjective::Pestle:
		break;

	case ECSKBotObjective::Wander:
	default:
	{
		const bool bReached = Memory->bHasMoveTarget && FVector::Dist2D(BotLocation, Memory->MoveTarget) <= ReachAcceptRadius;

		if (!Memory->bHasMoveTarget || bReached)
		{
			UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
			FNavLocation RandomPoint;
			if (NavSys && NavSys->GetRandomReachablePointInRadius(BotLocation, Config->DoughWanderRadius, RandomPoint))
			{
				Memory->MoveTarget = RandomPoint.Location;
				Memory->bHasMoveTarget = true;
			}
		}
		MoveTarget = Memory->bHasMoveTarget ? Memory->MoveTarget : BotLocation;
		break;
	}
	}

	// 혼란 중엔 목표가 점대칭으로 뒤집힘
	if (Bot->GetStatusComponent()->HasStatus(ECSKStatusType::Confusion))
	{
		MoveTarget = BotLocation * 2.0 - MoveTarget;
	}

	MoveTarget.Z = BotLocation.Z;

	// --- 블랙보드 기록 ---
	Blackboard->SetValueAsEnum(ObjectiveKey.SelectedKeyName, (uint8)Best);
	if (Best != ECSKBotObjective::Pestle)
	{
		Blackboard->SetValueAsVector(MoveTargetKey.SelectedKeyName, MoveTarget);
		Blackboard->SetValueAsObject(AttackTargetKey.SelectedKeyName, FightTarget);
	}

	// 디버그 오버레이용 스냅샷
	FCSKBotObjectiveDebugInfo DebugInfo;
	DebugInfo.bValid = true;
	DebugInfo.Current = Best;
	DebugInfo.RankPressure = RankPressure;
	for (int32 Index = 0; Index < ObjectiveCount; ++Index)
	{
		DebugInfo.Scores[Index] = Scores[Index];
	}
	AIController->SetObjectiveDebugInfo(DebugInfo);
}

// 도망 지점 탐색
FVector UBTService_SelectBotObjective::ComputeFleePoint(
	ACSKPlayerCharacter& Bot,
	const FVector& KingLocation,
	float FleeExitRange,
	const UCSKBotAIConfigData& Config,
	FObjectiveMemory& Memory) const
{
	const FVector BotLocation = Bot.GetActorLocation();
	const float DistToKing = FVector::Dist2D(BotLocation, KingLocation);

	const bool bTargetReached = Memory.bHasMoveTarget && FVector::Dist2D(BotLocation, Memory.MoveTarget) <= ReachAcceptRadius;
	const bool bTargetStillSafe = Memory.bHasMoveTarget && FVector::Dist2D(Memory.MoveTarget, KingLocation) >= FleeExitRange;

	if (Memory.bHasMoveTarget && !bTargetReached && bTargetStillSafe)
	{
		return Memory.MoveTarget;
	}

	FVector FleeDir = (BotLocation - KingLocation).GetSafeNormal2D();
	if (FleeDir.IsNearlyZero())
	{
		FleeDir = Bot.GetActorForwardVector().GetSafeNormal2D();
	}

	const float Need = (FleeExitRange + FleeBuffer) - DistToKing;
	const float Hop = FMath::Max(Config.DoughFleeDistance, Need);

	float BestScore = -TNumericLimits<float>::Max();
	FVector BestPoint = BotLocation;
	bool bFound = false;

	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Bot.GetWorld()))
	{
		for (const float Angle : FleeCandidateAngles)
		{
			const FVector Dir = FleeDir.RotateAngleAxis(Angle, FVector::UpVector);
			const FVector Candidate = BotLocation + Dir * Hop;

			FNavLocation Projected;
			if (NavSys->ProjectPointToNavigation(Candidate, Projected, NavProjectExtent))
			{
				const float DistKing = FVector::Dist2D(Projected.Location, KingLocation);
				if (DistKing <= DistToKing)
				{
					continue;
				}

				const float ClampDist = FVector::Dist2D(Projected.Location, Candidate);
				const float Score = DistKing - ClampDist * EdgePenaltyWeight;
				if (Score > BestScore)
				{
					BestScore = Score;
					BestPoint = Projected.Location;
					bFound = true;
				}
			}
		}
	}

	Memory.MoveTarget = bFound ? BestPoint : BotLocation;
	Memory.bHasMoveTarget = bFound;
	return Memory.MoveTarget;
}
