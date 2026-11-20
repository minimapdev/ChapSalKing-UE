#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_DodgeProjectile.generated.h"

class AShotProjectile;

UCLASS()
class CHAPSALKING_API UBTService_DodgeProjectile : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_DodgeProjectile();

protected:
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	struct FDodgeMemory
	{
		TWeakObjectPtr<AShotProjectile> Threat;
		float DodgeTime = 0.0f;
	};
};
