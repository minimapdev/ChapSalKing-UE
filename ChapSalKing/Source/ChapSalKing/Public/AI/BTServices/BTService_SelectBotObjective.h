#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "AI/CSKBotObjectiveTypes.h"
#include "BTService_SelectBotObjective.generated.h"

class ACSKPlayerCharacter;
class UCSKBotAIConfigData;

UCLASS()
class CHAPSALKING_API UBTService_SelectBotObjective : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SelectBotObjective();

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	struct FObjectiveMemory
	{
		ECSKBotObjective Current = ECSKBotObjective::Wander;
		float LastSwitchTime = -100.0f;
		FVector MoveTarget = FVector::ZeroVector;
		bool bHasMoveTarget = false;
		float PestleSeenTime = -1.0f;
	};

	FVector ComputeFleePoint(
		ACSKPlayerCharacter& Bot,
		const FVector& KingLocation,
		float FleeExitRange,
		const UCSKBotAIConfigData& Config,
		FObjectiveMemory& Mem) const;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector KingActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ObjectiveKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MoveTargetKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector AttackTargetKey;
};
