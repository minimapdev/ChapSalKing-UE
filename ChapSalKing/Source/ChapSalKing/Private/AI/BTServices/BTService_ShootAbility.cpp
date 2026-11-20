#include "AI/BTServices/BTService_ShootAbility.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

#include "AI/CSKBotAIController.h"
#include "AI/CSKBotAIConfigData.h"
#include "AI/CSKBotObjectiveTypes.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerShootComponent.h"
#include "Player/Components/CSKPlayerIngredientComponent.h"
#include "Player/Components/CSKPlayerStatusComponent.h"
#include "Ingredient/IngredientData.h"
#include "Props/Field/SmokeCloud.h"

UBTService_ShootAbility::UBTService_ShootAbility()
{
	NodeName = TEXT("Shoot Ability");
	bNotifyTick = true;
	bNotifyBecomeRelevant = true;
	Interval = 0.3f;
	RandomDeviation = 0.1f;
}

void UBTService_ShootAbility::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		ObjectiveKey.ResolveSelectedKey(*BBAsset);
		AttackTargetKey.ResolveSelectedKey(*BBAsset);
		KingActorKey.ResolveSelectedKey(*BBAsset);
	}
}

uint16 UBTService_ShootAbility::GetInstanceMemorySize() const
{
	return sizeof(FShootAbilityMemory);
}

void UBTService_ShootAbility::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	*reinterpret_cast<FShootAbilityMemory*>(NodeMemory) = FShootAbilityMemory();
}

void UBTService_ShootAbility::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ACSKBotAIController* AIController = Cast<ACSKBotAIController>(OwnerComp.GetAIOwner());
	ACSKPlayerCharacter* Bot = AIController ? Cast<ACSKPlayerCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Bot || !Blackboard || Bot->IsChapSalKing())
	{
		return;
	}

	// 봇 별 메모리를 꺼내 쿨다운 깎음
	FShootAbilityMemory* Memory = reinterpret_cast<FShootAbilityMemory*>(NodeMemory);
	Memory->TimeUntilNextShot -= DeltaSeconds;
	if (Memory->TimeUntilNextShot > 0.0f)
	{
		return;
	}

	// 인지 차단 체크 (인절미 연막 효과 등)
	if (Bot->GetStatusComponent()->HasStatus(ECSKStatusType::VisionBlocked))
	{
		return;
	}

	// 재료(탄약) 확인
	UCSKPlayerIngredientComponent* IngredientComp = Bot->GetIngredientComponent();
	UIngredientData* Ingredient = IngredientComp ? IngredientComp->GetCurrentIngredient() : nullptr;
	if (!Ingredient)
	{
		return;
	}

	// 목표별 사격 대상 설정
	const ECSKBotObjective Objective = (ECSKBotObjective)Blackboard->GetValueAsEnum(ObjectiveKey.SelectedKeyName);
	ACSKPlayerCharacter* Target = nullptr;

	if (Objective == ECSKBotObjective::Fight)	// 전투 상태인 봇은 전투 상대
	{
		Target = Cast<ACSKPlayerCharacter>(Blackboard->GetValueAsObject(AttackTargetKey.SelectedKeyName));
	}
	else if (Objective == ECSKBotObjective::Flee)	// 도망 상태인 봇은 찹쌀킹
	{
		Target = Cast<ACSKPlayerCharacter>(Blackboard->GetValueAsObject(KingActorKey.SelectedKeyName));
	}

	if (!IsValid(Target) || Target == Bot)
	{
		return;
	}

	const UCSKBotAIConfigData* Config = AIController->GetAIConfig();
	const FCSKBotPersonality& Personality = AIController->GetPersonality();

	// 발사 위치 
	USceneComponent* Muzzle = Bot->GetShootMuzzlePoint();
	UWorld* World = Bot->GetWorld();
	if (!Muzzle || !World)
	{
		return;
	}

	// 발사 위치와 타겟의 위치를 얻어 거리 계산
	const FVector MuzzleLocation = Muzzle->GetComponentLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	const float Dist = FVector::Dist(MuzzleLocation, TargetLocation);

	if (Dist > Config->ShootRange)	// 사거리
	{
		return;
	}

	// 시야 확인: 벽 또는 연막에 막힘
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Bot);
	FHitResult WallHit;

	if (World->LineTraceSingleByChannel(WallHit, MuzzleLocation, TargetLocation, ECC_Visibility, Params)
		|| ASmokeCloud::IsSightBlocked(World, MuzzleLocation, TargetLocation))
	{
		return;
	}

	// 예측샷
	FVector Predicted = TargetLocation;
	const float ProjectileSpeed = Ingredient->ShotParams.Speed;
	if (ProjectileSpeed > 0.0f)
	{
		const float Lead = FMath::Min(Config->LeadFactor * FMath::Lerp(0.6f, 1.2f, Personality.Accuracy), 1.0f);
		Predicted += Target->GetVelocity() * (Dist / ProjectileSpeed) * Lead;
	}

	// 조준 오차(원뿔 산포)
	const float ErrorHalfAngleDeg = Config->AimErrorHalfAngleDeg * FMath::Lerp(1.5f, 0.5f, Personality.Accuracy);
	const FVector Direction = FMath::VRandCone((Predicted - MuzzleLocation).GetSafeNormal(), FMath::DegreesToRadians(ErrorHalfAngleDeg));


	// 슈팅 & 성공시 쿨 다운. 실패시 다음 틱 재시도
	UCSKPlayerShootComponent* ShootComp = Bot->GetShootComponent();
	if (ShootComp && ShootComp->TryShootAt(MuzzleLocation + Direction * Dist))
	{
		Memory->TimeUntilNextShot = FMath::FRandRange(FMath::Max(Config->ShootInterval - Config->ShootIntervalRandom, 0.0f), 
			Config->ShootInterval + Config->ShootIntervalRandom) * Personality.TimingScale;			
	}
}
