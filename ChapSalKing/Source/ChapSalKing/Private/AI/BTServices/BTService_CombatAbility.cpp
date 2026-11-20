#include "AI/BTServices/BTService_CombatAbility.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Combat/CSKCombatAgentInterface.h"
#include "AI/CSKBotAIController.h"
#include "AI/CSKBotAIConfigData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerIngredientComponent.h"
#include "Ingredient/IngredientData.h"
#include "Combat/ShotProjectile.h"

UBTService_CombatAbility::UBTService_CombatAbility()
{
	NodeName = TEXT("Combat Ability");
	bNotifyTick = true;
	bNotifyBecomeRelevant = true;

	Interval = 0.5f;
	RandomDeviation = 0.0f;
}

void UBTService_CombatAbility::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		AttackTargetKey.ResolveSelectedKey(*BBAsset);
	}
}

uint16 UBTService_CombatAbility::GetInstanceMemorySize() const
{
	return sizeof(FCombatAbilityMemory);
}

void UBTService_CombatAbility::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	*reinterpret_cast<FCombatAbilityMemory*>(NodeMemory) = FCombatAbilityMemory();
}

void UBTService_CombatAbility::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	// 봇 별 메모리를 꺼내 쿨다운 깎음
	FCombatAbilityMemory* Memory = reinterpret_cast<FCombatAbilityMemory*>(NodeMemory);
	Memory->TimeUntilNextAttempt -= DeltaSeconds;
	if (Memory->TimeUntilNextAttempt > 0.0f)
	{
		return;  
	}

	ACSKBotAIController* AIController = Cast<ACSKBotAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return;
	}

	// Combat Agent 인터페이스 구현 확인함
	APawn* BotPawn = AIController->GetPawn();
	if (!BotPawn || !BotPawn->GetClass()->ImplementsInterface(UCSKCombatAgentInterface::StaticClass()))
	{
		return;
	}

	const ACSKPlayerCharacter* BotCharacter = Cast<ACSKPlayerCharacter>(BotPawn);
	if (BotCharacter && !BotCharacter->IsChapSalKing())
	{
		const UCSKPlayerIngredientComponent* IngredientComponent = BotCharacter->GetIngredientComponent();
		const UIngredientData* Ingredient =
			IngredientComponent ? IngredientComponent->GetCurrentIngredient() : nullptr;
		if (Ingredient && Ingredient->ProjectileClass)
		{
			return;
		}
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return;
	}

	// 공격 대상 확보
	AActor* AttackTarget = Cast<AActor>(Blackboard->GetValueAsObject(AttackTargetKey.SelectedKeyName));
	if (!AttackTarget)
	{
		return;   
	}

	AIController->SetFocus(AttackTarget);	// 공격 전 Focus 회전.

	// 전투 요청 실행 구조체
	FCSKCombatRequest Request;
	Request.ActionType = ActionType;
	const bool bAttacked = ICSKCombatAgentInterface::Execute_RequestCombatAction(BotPawn, Request);

	// 공격 성공시 쿨다운 세팅. 실패시 다음 틱 재시도
	if (bAttacked)
	{
		const UCSKBotAIConfigData* Config = AIController->GetAIConfig();
		const float Base = Config->AttackInterval;
		const float Rand = Config->AttackIntervalRandom;
		const float Scale = AIController->GetPersonality().TimingScale;
		Memory->TimeUntilNextAttempt = FMath::FRandRange(FMath::Max(Base - Rand, 0.0f), Base + Rand) * Scale;
	}
}
