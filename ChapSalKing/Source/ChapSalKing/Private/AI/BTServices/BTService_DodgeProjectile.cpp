#include "AI/BTServices/BTService_DodgeProjectile.h"
#include "EngineUtils.h"
#include "Engine/World.h"

#include "AI/CSKBotAIController.h"
#include "AI/CSKBotAIConfigData.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerMoveComponent.h"
#include "Player/Components/CSKPlayerStatusComponent.h"
#include "Combat/ShotProjectile.h"

UBTService_DodgeProjectile::UBTService_DodgeProjectile()
{
	NodeName = TEXT("Dodge Projectile");
	bNotifyTick = true;
	bNotifyBecomeRelevant = true;
	Interval = 0.1f;
	RandomDeviation = 0.02f;
}

uint16 UBTService_DodgeProjectile::GetInstanceMemorySize() const
{
	return sizeof(FDodgeMemory);
}

void UBTService_DodgeProjectile::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	*reinterpret_cast<FDodgeMemory*>(NodeMemory) = FDodgeMemory();
}

void UBTService_DodgeProjectile::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ACSKBotAIController* AIController = Cast<ACSKBotAIController>(OwnerComp.GetAIOwner());
	ACSKPlayerCharacter* Bot = AIController ? Cast<ACSKPlayerCharacter>(AIController->GetPawn()) : nullptr;

	// 찹쌀킹은 회피 X
	if (!Bot || Bot->IsChapSalKing())
	{
		return;
	}

	// 인지 차단 중에는 회피하지 못함
	if (Bot->GetStatusComponent()->HasStatus(ECSKStatusType::VisionBlocked))
	{
		return;
	}

	const UCSKBotAIConfigData* Config = AIController->GetAIConfig();
	const FCSKBotPersonality& Personality = AIController->GetPersonality();
	FDodgeMemory* Memory = reinterpret_cast<FDodgeMemory*>(NodeMemory);
	UWorld* World = Bot->GetWorld();
	const float Now = World->GetTimeSeconds();
	const FVector BotLocation = Bot->GetActorLocation();

	// 기억 중인 위협 투사체가 살아 있다면
	if (Memory->Threat.IsValid())
	{
		if (Now >= Memory->DodgeTime)
		{
			const FVector ThreatVelocity = Memory->Threat->GetVelocity(); // 투사체 속도 벡터
			const FVector ToBotNow = BotLocation - Memory->Threat->GetActorLocation(); // 투사체 방향 벡터
			// 투사체 속도 & 투사체가 향하는 방향과 투사체에서 봇으로의 방향 내적 판정
			const bool bStillIncoming = ThreatVelocity.SizeSquared() > 1.0f && 
				FVector::DotProduct(ThreatVelocity.GetSafeNormal(), ToBotNow.GetSafeNormal()) >= Config->DodgeConeDotThreshold;

			UCSKPlayerMoveComponent* Move = Bot->GetMoveComponent();

			if (bStillIncoming && Move && Move->CanDash()
				&& FMath::FRand() < Config->DodgeChance * Personality.Reflex)
			{
				const FVector ProjDir = Memory->Threat->GetVelocity().GetSafeNormal2D();	// 투사체 XY정규화. 수평 성분만

				// 투사체 진행방향과 UP벡터의 외적 → 투사체에 직각인 수평 옆방향. 
				FVector Side = FVector::CrossProduct(ProjDir, FVector::UpVector);	
				if (FMath::RandBool())
				{
					Side = -Side; // 오른쪽 왼쪽 랜덤 선택
				}
				Move->TryDash(Side);
			}
			Memory->Threat.Reset();
		}
		return;
	}

	const float DetectRangeSq = FMath::Square(Config->DodgeDetectRange);

	// 새로운 위협 투사체 탐지.
	for (TActorIterator<AShotProjectile> It(World); It; ++It)
	{
		AShotProjectile* Projectile = *It;
		if (Projectile->GetOwner() == Bot)
		{
			continue;
		}

		// 투사체 -> 봇 방향 벡터
		const FVector ToBot = BotLocation - Projectile->GetActorLocation();
		if (ToBot.SizeSquared() > DetectRangeSq)
		{
			continue;
		}

		// 투사체 속도 벡터
		const FVector Velocity = Projectile->GetVelocity();
		if (Velocity.SizeSquared() < 1.0f
			|| FVector::DotProduct(Velocity.GetSafeNormal(), ToBot.GetSafeNormal()) < Config->DodgeConeDotThreshold)
		{
			continue;
		}

		Memory->Threat = Projectile;
		Memory->DodgeTime = Now + Config->DodgeReactionDelay * FMath::Lerp(1.4f, 0.7f, Personality.Reflex);
		break;
	}
}
