#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_ShootAbility.generated.h"

UCLASS()
class CHAPSALKING_API UBTService_ShootAbility : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_ShootAbility();

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	struct FShootAbilityMemory
	{
		float TimeUntilNextShot = 0.0f;
	};

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ObjectiveKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector AttackTargetKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector KingActorKey;
};
