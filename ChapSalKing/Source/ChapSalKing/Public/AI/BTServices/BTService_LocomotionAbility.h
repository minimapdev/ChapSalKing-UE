#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_LocomotionAbility.generated.h"

UCLASS()
class CHAPSALKING_API UBTService_LocomotionAbility : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_LocomotionAbility();

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MoveTargetKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SelfIsKingKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector KingActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ChaseTargetKey;
};
