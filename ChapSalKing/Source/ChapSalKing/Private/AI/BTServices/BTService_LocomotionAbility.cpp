#include "AI/BTServices/BTService_LocomotionAbility.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"
#include "AI/CSKBotAIController.h"
#include "AI/CSKBotAIConfigData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerMoveComponent.h"

UBTService_LocomotionAbility::UBTService_LocomotionAbility()
{
	NodeName = TEXT("Locomotion Ability");
	bNotifyTick = true;

	Interval = 0.3f;
	RandomDeviation = 0.1f;
}

void UBTService_LocomotionAbility::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		MoveTargetKey.ResolveSelectedKey(*BBAsset);
		SelfIsKingKey.ResolveSelectedKey(*BBAsset);
		KingActorKey.ResolveSelectedKey(*BBAsset);
		ChaseTargetKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_LocomotionAbility::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

	UCSKPlayerMoveComponent* Move = Bot->GetMoveComponent();
	if (!Move)
	{
		return;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return;
	}

	// Config 로드
	const UCSKBotAIConfigData* Config = AIController->GetAIConfig();
	const float DashChanceNear = Config->DashChanceNear;
	const float DashChanceFar = Config->DashChanceFar;
	const float DashFarDistance = Config->DashFarDistance;
	const float ThreatRange = Config->ThreatRange;
	const float EscapeDashChance = Config->EscapeDashChance;
	const float UrgencyDistance = Config->LocomotionUrgencyDistance;

	// 찹쌀킹 정보 읽기
	const bool bSelfIsKing = Blackboard->GetValueAsBool(SelfIsKingKey.SelectedKeyName);
	const ACSKPlayerCharacter* CurrentKing = Cast<ACSKPlayerCharacter>(Blackboard->GetValueAsObject(KingActorKey.SelectedKeyName));

	const FVector BotLocation = Bot->GetActorLocation();
	float ContextMultiplier = 1.0f;	// 상황에 따라 점프 & 대쉬를 더 자주하게 만드는 배율

	if (bSelfIsKing)	// 내가 찹쌀킹
	{
		const ACSKPlayerCharacter* ChaseTarget = Cast<ACSKPlayerCharacter>(Blackboard->GetValueAsObject(ChaseTargetKey.SelectedKeyName));

		if (ChaseTarget)
		{
			// 추격 대상과 가까울수록 배율 증가
			const float Dist = FVector::Dist2D(BotLocation, ChaseTarget->GetActorLocation());
			ContextMultiplier = FMath::Lerp(2.0f, 1.0f, FMath::Clamp(Dist / UrgencyDistance, 0.0f, 1.0f));
		}
	}
	else if (CurrentKing)	// 현재 찹쌀킹이 존재(나는 반죽)
	{
		// 찹쌀킹과 가까울수록 배율 증가
		const float Dist = FVector::Dist2D(BotLocation, CurrentKing->GetActorLocation());
		ContextMultiplier = FMath::Lerp(2.5f, 1.0f, FMath::Clamp(Dist / UrgencyDistance, 0.0f, 1.0f));
	}

	// 점프
	const FVector MoveTarget = Blackboard->GetValueAsVector(MoveTargetKey.SelectedKeyName);
	const float JumpRate = Config->JumpChance * AIController->GetPersonality().JumpScale * ContextMultiplier;

	if (FMath::FRand() < JumpRate * DeltaSeconds)
	{
		Move->TryJump();
	}

	// 대쉬
	if (!Move->CanDash())
	{
		return;
	}

	const FVector ToTarget = MoveTarget - BotLocation;
	const float Alpha = FMath::Clamp(ToTarget.Size2D() / DashFarDistance, 0.0f, 1.0f);
	float DashRate = FMath::Lerp(DashChanceNear, DashChanceFar, Alpha) * ContextMultiplier;
	FVector DashDir = ToTarget;

	// 절구 공이 가지고 있으면 가까운 상대 찾고 대쉬 TODO:이것도 나중에 GameState에서 목록 받아오기
	if (Bot->GetCarriedPestleItem())
	{
		float NearestDist = TNumericLimits<float>::Max();
		for (TActorIterator<ACSKPlayerCharacter> It(Bot->GetWorld()); It; ++It)
		{
			ACSKPlayerCharacter* Other = *It;
			if (!Other || Other == Bot)
			{
				continue;
			}
			NearestDist = FMath::Min(NearestDist, FVector::Dist2D(BotLocation, Other->GetActorLocation()));
		}
		if (NearestDist < ThreatRange)
		{
			DashRate += EscapeDashChance * (1.0f - NearestDist / ThreatRange);
		}
	}

	DashRate *= AIController->GetPersonality().DashScale;
	if (DashRate > 0.0f && FMath::FRand() < DashRate * DeltaSeconds)
	{
		Move->TryDash(DashDir);
	}
}
