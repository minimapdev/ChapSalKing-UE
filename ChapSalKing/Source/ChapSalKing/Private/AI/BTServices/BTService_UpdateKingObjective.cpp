#include "AI/BTServices/BTService_UpdateKingObjective.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"
#include "AI/CSKBotAIController.h"
#include "AI/CSKBotAIConfigData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerStatusComponent.h"

UBTService_UpdateKingObjective::UBTService_UpdateKingObjective()
{
	NodeName = TEXT("Update King Objective");
	bNotifyTick = true;
	Interval = 0.15f;
	RandomDeviation = 0.0f;
}

void UBTService_UpdateKingObjective::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		AttackTargetKey.ResolveSelectedKey(*BBAsset);
		MoveTargetKey.ResolveSelectedKey(*BBAsset);
		ChaseTargetKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_UpdateKingObjective::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ACSKBotAIController* AIController = Cast<ACSKBotAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return;
	}

	ACSKPlayerCharacter* Bot = Cast<ACSKPlayerCharacter>(AIController->GetPawn());
	if (!Bot)
	{
		return;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return;
	}

	// 인지 차단 중엔 시각 타겟팅 불가
	if (Bot->GetStatusComponent()->HasStatus(ECSKStatusType::VisionBlocked))
	{
		Blackboard->SetValueAsObject(ChaseTargetKey.SelectedKeyName, nullptr);
		Blackboard->SetValueAsObject(AttackTargetKey.SelectedKeyName, nullptr);
		Blackboard->SetValueAsVector(MoveTargetKey.SelectedKeyName, Bot->GetActorLocation());
		return;
	}

	// Config 로드
	const UCSKBotAIConfigData* Config = AIController->GetAIConfig();
	const float Aggression = AIController->GetPersonality().Aggression;

	const float EatRange = Config->KingEatRange;
	const float FacingWeight = Config->KingFacingScoreWeight;

	const float GiveUpRange = FMath::Lerp(Config->KingGiveUpRangeMin, Config->KingGiveUpRangeMax, Aggression);
	const float SwitchMargin = FMath::Lerp(Config->KingTargetSwitchMarginMin, Config->KingTargetSwitchMarginMax, Aggression);

	const FVector BotLocation = Bot->GetActorLocation();
	const FVector BotForward = Bot->GetActorForwardVector().GetSafeNormal2D();

	// 반죽을 지금 쫓을 매력도 점수화
	auto ScoreDough = [&](const ACSKPlayerCharacter* D) -> float
		{
			const FVector ToD = D->GetActorLocation() - BotLocation;	// 찹쌀킹 -> 반죽 벡터
			const float Dist = ToD.Size2D();
			const float Facing = FVector::DotProduct(BotForward, ToD.GetSafeNormal2D());
			return -Dist + FacingWeight * Facing;
		};

	// 현재 타겟 유효성 평가
	ACSKPlayerCharacter* CurrentTarget = Cast<ACSKPlayerCharacter>(Blackboard->GetValueAsObject(ChaseTargetKey.SelectedKeyName));

	bool bCurrentValid = false;
	float CurrentScore = TNumericLimits<float>::Lowest();
	if (CurrentTarget && !CurrentTarget->IsChapSalKing() && !CurrentTarget->IsBeingEaten())
	{
		const float Dist = FVector::Dist2D(BotLocation, CurrentTarget->GetActorLocation());
		if (Dist <= GiveUpRange)
		{
			bCurrentValid = true;
			CurrentScore = ScoreDough(CurrentTarget);
		}
	}

	// 최고 후보 탐색
	ACSKPlayerCharacter* BestCandidate = nullptr;
	float BestScore = TNumericLimits<float>::Lowest();
	for (TActorIterator<ACSKPlayerCharacter> It(Bot->GetWorld()); It; ++It)	// TODO: GameState에서 받아오게. 
	{
		ACSKPlayerCharacter* Other = *It;
		if (!Other || Other == Bot || Other->IsChapSalKing() || Other->IsBeingEaten())
		{
			continue;
		}

		const float S = ScoreDough(Other);
		if (S > BestScore)
		{
			BestScore = S;
			BestCandidate = Other;
		}
	}

	// 타겟 결정
	ACSKPlayerCharacter* Target = nullptr;
	if (bCurrentValid)
	{
		Target = (BestCandidate && BestScore > CurrentScore + SwitchMargin) ? BestCandidate : CurrentTarget;
	}
	else
	{
		Target = BestCandidate;
	}

	Blackboard->SetValueAsObject(ChaseTargetKey.SelectedKeyName, Target);

	// 포식 판정 + 이동
	ACSKPlayerCharacter* AttackTarget = nullptr;
	FVector MoveTarget = BotLocation;
	if (Target)
	{
		MoveTarget = Target->GetActorLocation();
		const float Dist = FVector::Dist2D(BotLocation, Target->GetActorLocation());
		if (Dist <= EatRange)
		{
			AttackTarget = Target;
		}
	}

	Blackboard->SetValueAsObject(AttackTargetKey.SelectedKeyName, AttackTarget);

	// 혼란 중엔 목표 점대칭 반전
	if (Bot->GetStatusComponent()->HasStatus(ECSKStatusType::Confusion))
	{
		MoveTarget = BotLocation * 2.0 - MoveTarget;
	}

	MoveTarget.Z = BotLocation.Z;
	Blackboard->SetValueAsVector(MoveTargetKey.SelectedKeyName, MoveTarget);
}
