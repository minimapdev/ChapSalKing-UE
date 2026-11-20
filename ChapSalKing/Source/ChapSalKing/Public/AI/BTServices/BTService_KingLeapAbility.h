#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_KingLeapAbility.generated.h"

UCLASS()
class CHAPSALKING_API UBTService_KingLeapAbility : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_KingLeapAbility();

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	struct FKingLeapMemory
	{
		float TimeUntilNextAttempt = 0.0f;
	};

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ChaseTargetKey;
};
