#include "AI/BTServices/BTService_StuckRecovery.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/CSKBotAIController.h"
#include "AI/CSKBotAIConfigData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerMoveComponent.h"

UBTService_StuckRecovery::UBTService_StuckRecovery()
{
	NodeName = TEXT("Stuck Recovery");
	bNotifyTick = true;
	bNotifyBecomeRelevant = true;
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_StuckRecovery::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		MoveTargetKey.ResolveSelectedKey(*BBAsset);
	}
}

uint16 UBTService_StuckRecovery::GetInstanceMemorySize() const
{
	return sizeof(FStuckMemory);
}

void UBTService_StuckRecovery::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	*reinterpret_cast<FStuckMemory*>(NodeMemory) = FStuckMemory();
}

void UBTService_StuckRecovery::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ACSKBotAIController* AIController = Cast<ACSKBotAIController>(OwnerComp.GetAIOwner());
	ACSKPlayerCharacter* Bot = AIController ? Cast<ACSKPlayerCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Bot || !Blackboard)
	{
		return;
	}

	const UCSKBotAIConfigData* Config = AIController->GetAIConfig();
	FStuckMemory* Memory = reinterpret_cast<FStuckMemory*>(NodeMemory);
	const FVector BotLocation = Bot->GetActorLocation();
	const FVector MoveTarget = Blackboard->GetValueAsVector(MoveTargetKey.SelectedKeyName);

	const float Moved = FVector::Dist2D(BotLocation, Memory->LastLocation);
	Memory->LastLocation = BotLocation;

	// 정상 이동 or 목표도착 or 행동 불능
	if (Moved > Config->StuckMoveTolerance
		|| FVector::Dist2D(BotLocation, MoveTarget) < Config->StuckGoalIgnoreRadius
		|| Bot->IsInputLocked())
	{
		Memory->StuckSeconds = 0.0f;
		return;
	}

	// Stuck 상태 업데이트
	Memory->StuckSeconds += DeltaSeconds;
	if (Memory->StuckSeconds < Config->StuckSecondsThreshold)
	{
		return;
	}
	Memory->StuckSeconds = 0.0f;

	// Stuck 탈출 : 목표 방향 점프+대시
	if (UCSKPlayerMoveComponent* Move = Bot->GetMoveComponent())
	{
		const FVector Direction = (MoveTarget - BotLocation).GetSafeNormal2D();
		Move->TryJump();
		if (Move->CanDash())
		{
			Move->TryDash(Direction);
		}
	}
}
