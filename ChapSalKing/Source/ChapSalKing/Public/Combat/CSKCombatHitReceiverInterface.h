#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Combat/CSKCombatTypes.h"
#include "CSKCombatHitReceiverInterface.generated.h"

UINTERFACE(BlueprintType)
class CHAPSALKING_API UCSKCombatHitReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

class CHAPSALKING_API ICSKCombatHitReceiverInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CSK|Combat")
	void ReceiveCombatHit(const FCSKCombatHitContext& HitContext);
};
