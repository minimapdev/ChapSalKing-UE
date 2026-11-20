#include "AI/CSKBotAIController.h"
#include "EngineUtils.h"
#include "AI/CSKBotAIConfigData.h"
#include "Framework/CSKGameState.h"
#include "Player/CSKPlayerCharacter.h"
#include "BrainComponent.h"
#include "TimerManager.h"
#include "Props/Mortar/Seesaw.h"
#include "Props/Mortar/PestleItem.h"
#include "Props/Cauldron/Cauldron.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

ACSKBotAIController::ACSKBotAIController()
{
	bWantsPlayerState = true;
}

void ACSKBotAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	checkf(AIConfig, TEXT("CSKBotAIController의 AIConfig가 설정되지 않았습니다."));

	Personality = AIConfig->RollPersonality();

	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}

	SubscribeToKingChanged();
}

void ACSKBotAIController::OnUnPossess()
{
	UnsubscribeFromKingChanged();
	GetWorldTimerManager().ClearTimer(StaggerTimerHandle);
	LogicPauseReasons.Reset();

	Super::OnUnPossess();

	if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent))
	{
		BTComp->StopTree();
	}
}

void ACSKBotAIController::SubscribeToKingChanged()
{
	if (ACSKGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ACSKGameState>() : nullptr)
	{
		KingChangedHandle = GameState->OnChapSalKingChanged.AddUObject(
			this, &ACSKBotAIController::HandleKingChanged);

		// 구독 시점의 현재 킹 즉시 반영
		HandleKingChanged(GameState->GetChapSalKing());
	}
}

void ACSKBotAIController::UnsubscribeFromKingChanged()
{
	if (ACSKGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ACSKGameState>() : nullptr)
	{
		GameState->OnChapSalKingChanged.Remove(KingChangedHandle);
	}
	KingChangedHandle.Reset();
}

void ACSKBotAIController::HandleKingChanged(ACSKPlayerCharacter* NewKing)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	const APawn* SelfPawn = GetPawn();
	const bool bSelfIsKing = (NewKing != nullptr && NewKing == SelfPawn);

	BB->SetValueAsObject(CSKBlackboardKeys::KingActor, NewKing);
	BB->SetValueAsBool(CSKBlackboardKeys::SelfIsKing, bSelfIsKing);
}

ASeesaw* ACSKBotAIController::GetSeesaw()
{
	if (!CachedSeesaw)
	{
		for (TActorIterator<ASeesaw> It(GetWorld()); It; ++It)
		{
			CachedSeesaw = *It;
			break;
		}
	}
	return CachedSeesaw;
}

const TArray<TObjectPtr<ACauldron>>& ACSKBotAIController::GetCauldrons()
{
	if (!bCauldronsCached)
	{
		for (TActorIterator<ACauldron> It(GetWorld()); It; ++It)
		{
			CachedCauldrons.Add(*It);
		}
		bCauldronsCached = true;
	}
	return CachedCauldrons;
}

APestleItem* ACSKBotAIController::GetPestleItem()
{
	if (!IsValid(CachedPestleItem))
	{
		CachedPestleItem = nullptr;
		for (TActorIterator<APestleItem> It(GetWorld()); It; ++It)
		{
			CachedPestleItem = *It;
			break;
		}
	}
	return CachedPestleItem;
}

void ACSKBotAIController::ApplyStagger()
{
	StopMovement();
	PushLogicPause(CSKLogicPauseReasons::Staggered);

	const float Duration = AIConfig->StaggerDuration;

	GetWorldTimerManager().SetTimer(
		StaggerTimerHandle,
		this,
		&ACSKBotAIController::ClearStagger,
		Duration,
		false);
}

void ACSKBotAIController::ClearStagger()
{
	PopLogicPause(CSKLogicPauseReasons::Staggered);
}

void ACSKBotAIController::PushLogicPause(FName Reason)
{
	if (!BrainComponent || Reason.IsNone())
	{
		return;
	}

	if (LogicPauseReasons.Num() == 0)
	{
		BrainComponent->PauseLogic(Reason.ToString());
	}

	LogicPauseReasons.Add(Reason);
}

void ACSKBotAIController::PopLogicPause(FName Reason)
{
	if (!BrainComponent || LogicPauseReasons.Remove(Reason) == 0)
	{
		return;
	}

	if (LogicPauseReasons.Num() == 0)
	{
		BrainComponent->ResumeLogic(Reason.ToString());
	}
}
