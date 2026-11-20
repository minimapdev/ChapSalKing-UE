#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/CSKBotAIConfigData.h"
#include "AI/CSKBotObjectiveTypes.h"
#include "CSKBotAIController.generated.h"

class UBehaviorTree;
class ASeesaw;
class APestleItem;
class ACauldron;
class ACSKPlayerCharacter;

UCLASS()
class CHAPSALKING_API ACSKBotAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACSKBotAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UCSKBotAIConfigData* GetAIConfig() const
	{
		return AIConfig;
	}

	ASeesaw* GetSeesaw();
	APestleItem* GetPestleItem();
	// 월드의 가마솥 전부. 레벨에 고정 배치되므로 1회 수집
	const TArray<TObjectPtr<ACauldron>>& GetCauldrons();
	void ApplyStagger();

	// BrainComponent의 PauseLogic/ResumeLogic은 카운트가 없어서, 정지 사유가 겹치면
	// 먼저 풀린 쪽이 나머지 사유까지 재개시킨다. 사유 집합으로 소유권을 일원화한다.
	void PushLogicPause(FName Reason);
	void PopLogicPause(FName Reason);

	const FCSKBotPersonality& GetPersonality() const
	{
		return Personality;
	}

	void SetObjectiveDebugInfo(const FCSKBotObjectiveDebugInfo& InInfo)
	{
		ObjectiveDebugInfo = InInfo;
	}

	const FCSKBotObjectiveDebugInfo& GetObjectiveDebugInfo() const
	{
		return ObjectiveDebugInfo;
	}

private:
	void SubscribeToKingChanged();
	void UnsubscribeFromKingChanged();
	void HandleKingChanged(ACSKPlayerCharacter* NewKing);

	FDelegateHandle KingChangedHandle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Bot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKBotAIConfigData> AIConfig;

	UPROPERTY()
	TObjectPtr<ASeesaw> CachedSeesaw;

	UPROPERTY()
	TObjectPtr<APestleItem> CachedPestleItem;

	UPROPERTY()
	TArray<TObjectPtr<ACauldron>> CachedCauldrons;
	bool bCauldronsCached = false;

	void ClearStagger();
	FTimerHandle StaggerTimerHandle;
	TSet<FName> LogicPauseReasons;
	FCSKBotPersonality Personality;
	FCSKBotObjectiveDebugInfo ObjectiveDebugInfo;
};
