#include "AI/BTServices/BTService_KingLeapAbility.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"

#include "AI/CSKBotAIController.h"
#include "AI/CSKBotAIConfigData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerLeapComponent.h"
#include "Player/Components/CSKPlayerStatusComponent.h"

UBTService_KingLeapAbility::UBTService_KingLeapAbility()
{
	NodeName = TEXT("King Leap Ability");
	bNotifyTick = true;
	bNotifyBecomeRelevant = true;
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_KingLeapAbility::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		ChaseTargetKey.ResolveSelectedKey(*BBAsset);
	}
}

uint16 UBTService_KingLeapAbility::GetInstanceMemorySize() const
{
	return sizeof(FKingLeapMemory);
}

void UBTService_KingLeapAbility::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	*reinterpret_cast<FKingLeapMemory*>(NodeMemory) = FKingLeapMemory();
}

void UBTService_KingLeapAbility::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ACSKBotAIController* AIController = Cast<ACSKBotAIController>(OwnerComp.GetAIOwner());
	ACSKPlayerCharacter* Bot = AIController ? Cast<ACSKPlayerCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	// 도약은 찹쌀킹만 가능함.
	if (!Bot || !Blackboard || !Bot->IsChapSalKing())
	{
		return;
	}

	// 쿨다운.
	FKingLeapMemory* Memory = reinterpret_cast<FKingLeapMemory*>(NodeMemory);
	Memory->TimeUntilNextAttempt -= DeltaSeconds;
	if (Memory->TimeUntilNextAttempt > 0.0f)
	{
		return;
	}

	// 이미 도약 중 체크
	UCSKPlayerLeapComponent* Leap = Bot->GetLeapComponent();
	if (!Leap || Leap->IsLeapAttackActive())
	{
		return;
	}

	// 인지 차단 및 혼란 중엔 조준 도약 불가
	UCSKPlayerStatusComponent* Status = Bot->GetStatusComponent();
	if (Status->HasStatus(ECSKStatusType::VisionBlocked) || Status->HasStatus(ECSKStatusType::Confusion))
	{
		return;
	}

	const UCSKBotAIConfigData* Config = AIController->GetAIConfig();
	const FCSKBotPersonality& Personality = AIController->GetPersonality();
	UWorld* World = Bot->GetWorld();
	const FVector BotLocation = Bot->GetActorLocation();
	const float MaxRange = Leap->GetMaxLeapRange();

	// Blackboard에서 현재 추격 대상을 읽음.
	ACSKPlayerCharacter* ChaseTarget = Cast<ACSKPlayerCharacter>(Blackboard->GetValueAsObject(ChaseTargetKey.SelectedKeyName));

	// 포식 직전이면 도약 X
	if (IsValid(ChaseTarget) && FVector::Dist2D(BotLocation, ChaseTarget->GetActorLocation()) < Config->KingEatRange * 2.0f)
	{
		return;
	}

	// 도약할 후보 관련 변수
	float BestScore = 0.0f;
	FVector BestPoint = FVector::ZeroVector;

	// 도주자가 멀수록 도약 가치가 큼
	if (IsValid(ChaseTarget))
	{
		const float ChaseDist = FVector::Dist2D(BotLocation, ChaseTarget->GetActorLocation());
		if (ChaseDist >= Config->KingLeapMinDistance && ChaseDist <= MaxRange)
		{
			BestScore = (ChaseDist / MaxRange) * Config->KingLeapChaseWeight;
			BestPoint = ChaseTarget->GetActorLocation() + ChaseTarget->GetVelocity() * Config->KingLeapPredictSeconds * FMath::Lerp(0.6f, 1.2f, Personality.Accuracy);	// 예측샷
		}
	}

	// 밀집한 무리 가치 큼. 일단 지금은 임시로 월드 전부 훑음. TODO : 이거 GameState에서 미리 받아오기
	TArray<ACSKPlayerCharacter*> Doughs;
	for (TActorIterator<ACSKPlayerCharacter> It(World); It; ++It)
	{
		ACSKPlayerCharacter* Other = *It;
		if (IsValid(Other) && Other != Bot && !Other->IsChapSalKing() && !Other->IsBeingEaten())
		{
			Doughs.Add(Other);
		}
	}

	int32 BestCount = 1;
	FVector BestClusterCenter = FVector::ZeroVector;
	for (ACSKPlayerCharacter* Anchor : Doughs)
	{
		const float AnchorDist = FVector::Dist2D(BotLocation, Anchor->GetActorLocation());
		if (AnchorDist < Config->KingLeapMinDistance || AnchorDist > MaxRange)
		{
			continue;
		}

		int32 Count = 0;
		FVector Sum = FVector::ZeroVector;
		for (ACSKPlayerCharacter* Other : Doughs)
		{
			if (FVector::Dist2D(Anchor->GetActorLocation(), Other->GetActorLocation()) <= Config->KingLeapClusterRadius)
			{
				++Count;
				Sum += Other->GetActorLocation();
			}
		}

		if (Count > BestCount)
		{
			BestCount = Count;
			BestClusterCenter = Sum / Count;
		}
	}

	if (BestCount >= 2)
	{
		const float CenterDist = FVector::Dist2D(BotLocation, BestClusterCenter);
		if (CenterDist >= Config->KingLeapMinDistance && CenterDist <= MaxRange)
		{
			const float ClusterScore = BestCount * Config->KingLeapClusterScorePerDough;
			if (ClusterScore > BestScore)
			{
				BestScore = ClusterScore;
				BestPoint = BestClusterCenter;
			}
		}
	}

	if (BestScore <= 0.0f)
	{
		return;
	}

	if (FMath::FRand() > Config->KingLeapChance * Personality.JumpScale)
	{
		Memory->TimeUntilNextAttempt = Config->KingLeapDecisionInterval;
		return;
	}

	if (Leap->TryBotLeap(BestPoint))
	{
		Memory->TimeUntilNextAttempt = Config->KingLeapDecisionInterval;
	}
}
