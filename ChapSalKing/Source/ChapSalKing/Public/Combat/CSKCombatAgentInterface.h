#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Combat/CSKCombatRequestTypes.h"
#include "CSKCombatAgentInterface.generated.h"

UINTERFACE(BlueprintType)
class CHAPSALKING_API UCSKCombatAgentInterface : public UInterface
{
	GENERATED_BODY()
};

class CHAPSALKING_API ICSKCombatAgentInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CSK|Combat")
	bool RequestCombatAction(const FCSKCombatRequest& Request);
};
