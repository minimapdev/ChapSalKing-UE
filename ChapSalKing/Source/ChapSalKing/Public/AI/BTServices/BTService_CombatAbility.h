#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Combat/CSKCombatRequestTypes.h"
#include "BTService_CombatAbility.generated.h"

UCLASS()
class CHAPSALKING_API UBTService_CombatAbility : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_CombatAbility();

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	struct FCombatAbilityMemory
	{
		float TimeUntilNextAttempt = 0.0f;
	};

	UPROPERTY(EditAnywhere, Category = "CSK|Combat")
	ECSKCombatActionType ActionType = ECSKCombatActionType::BasicAttack;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector AttackTargetKey;		
};
