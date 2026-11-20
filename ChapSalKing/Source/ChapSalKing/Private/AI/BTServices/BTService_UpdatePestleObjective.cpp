#include "AI/BTServices/BTService_UpdatePestleObjective.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"
#include "AI/CSKBotAIController.h"
#include "AI/CSKBotAIConfigData.h"
#include "Props/Mortar/PestleItem.h"
#include "Props/Mortar/Seesaw.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerStatusComponent.h"

UBTService_UpdatePestleObjective::UBTService_UpdatePestleObjective()
{
	NodeName = TEXT("Update Pestle Objective");
	bNotifyTick = true;
}

void UBTService_UpdatePestleObjective::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		AttackTargetKey.ResolveSelectedKey(*BBAsset);
		MoveTargetKey.ResolveSelectedKey(*BBAsset);
		TargetPestleKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_UpdatePestleObjective::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ACSKBotAIController* AIController = Cast<ACSKBotAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return;
	}

	ACSKPlayerCharacter* BotCharacter = Cast<ACSKPlayerCharacter>(AIController->GetPawn());
	if (!BotCharacter)
	{
		return;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return;
	}

	const float AttackRange = AIController->GetAIConfig()->AttackRange;
	const FVector BotLocation = BotCharacter->GetActorLocation();

	// 내가 들고 있거나 절구에 꽂혀 있으면 추적 대상 제외
	APestleItem* BestPestle = AIController->GetPestleItem();
	if (BestPestle && (BestPestle->IsCarriedBy(BotCharacter) || BestPestle->IsPlanted()))
	{
		BestPestle = nullptr;
	}

	// 공격 대상 탐색
	ACSKPlayerCharacter* BestAttackTarget = nullptr;
	float BestAttackDist = AttackRange;

	for (TActorIterator<ACSKPlayerCharacter> It(AIController->GetWorld()); It; ++It)	// TODO : GameState에서 받아오게
	{
		ACSKPlayerCharacter* Other = *It;
		if (!Other || Other == BotCharacter || Other->IsBeingEaten())
		{
			continue;
		}

		const float Dist = FVector::Dist2D(BotLocation, Other->GetActorLocation());
		if (Dist <= AttackRange && Dist < BestAttackDist)
		{
			BestAttackDist = Dist;
			BestAttackTarget = Other;
		}
	}

	// 이둥 목표
	FVector MoveTarget = BotLocation;

	if (BotCharacter->GetCarriedPestleItem())	// 내가 들고 있으면 널뛰기로 운반
	{
		if (ASeesaw* Seesaw = AIController->GetSeesaw())
		{
			MoveTarget = Seesaw->GetActorLocation();
		}
	}
	else if (BestPestle)	// 절구 공이 누군가 운반 또는 땅에
	{
		const ACSKPlayerCharacter* Carrier = BestPestle->GetCarrierCharacter();
		MoveTarget = Carrier
			? Carrier->GetActorLocation() + AIController->GetPersonality().SurroundOffset	// 운반자 포위
			: BestPestle->GetActorLocation();	// 주우러
	}

	// 혼란 중엔 목표가 점대칭으로 뒤집힘
	if (BotCharacter->GetStatusComponent()->HasStatus(ECSKStatusType::Confusion))
	{
		MoveTarget = BotLocation * 2.0 - MoveTarget;
	}

	Blackboard->SetValueAsObject(TargetPestleKey.SelectedKeyName, BestPestle);
	Blackboard->SetValueAsObject(AttackTargetKey.SelectedKeyName, BestAttackTarget);
	MoveTarget.Z = BotLocation.Z;
	Blackboard->SetValueAsVector(MoveTargetKey.SelectedKeyName, MoveTarget);
}
