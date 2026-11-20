#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_StuckRecovery.generated.h"

UCLASS()
class CHAPSALKING_API UBTService_StuckRecovery : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_StuckRecovery();

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	struct FStuckMemory
	{
		FVector LastLocation = FVector::ZeroVector;
		float StuckSeconds = 0.0f;
	};

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MoveTargetKey;
};
